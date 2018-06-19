#include "Elements.h"
#include "Format.h"

#include <math.h>

// TODO:NEXT-VER in general case all parameters units should be verified too.
// But currenly we have verification only in that places which always use correct uints (e.g. Element props dialog).

class CurvatureRadiusVerifier : public Z::ValueVerifier
{
public:
    bool enabled() const override { return true; }
    QString verify(const Z::Value& value) const override
    {
        if (value.value() == 0)
            return qApp->translate("Param", "Curvature radius can not be zero.");
        return QString();
    }
};

class FocalLengthVerifier : public Z::ValueVerifier
{
public:
    bool enabled() const override { return true; }
    QString verify(const Z::Value& value) const override
    {
        if (value.value() == 0)
            return qApp->translate("Param", "Focal length can not be zero.");
        return QString();
    }
};

CurvatureRadiusVerifier* globalCurvatureRadiusVerifier()
{
    static CurvatureRadiusVerifier v; return &v;
}

FocalLengthVerifier* globalFocalLengthVerifier()
{
    static FocalLengthVerifier v; return &v;
}

//------------------------------------------------------------------------------
//                             ElemEmptyRange

void ElemEmptyRange::calcMatrixInternal()
{
    _mt.assign(1, lengthSI(), 0, 1);
    _ms = _mt;
}

void ElemEmptyRange::setSubRangeSI(double value)
{
    _mt1.assign(1, value, 0, 1);
    _ms1 = _mt1;
    _mt2.assign(1, lengthSI() - value, 0, 1);
    _ms2 = _mt2;
}

//------------------------------------------------------------------------------
//                             ElemMediaRange

ElemMediumRange::ElemMediumRange() : ElementRange()
{
    _ior->setVisible(true);
}

void ElemMediumRange::calcMatrixInternal()
{
    _mt.assign(1, lengthSI(), 0, 1);
    _ms = _mt;
}

void ElemMediumRange::setSubRangeSI(double value)
{
    _mt1.assign(1, value, 0, 1);
    _ms1 = _mt1;
    _mt2.assign(1, lengthSI() - value, 0, 1);
    _ms2 = _mt2;
}

//------------------------------------------------------------------------------
//                                ElemPlate

ElemPlate::ElemPlate() : ElementRange()
{
    _ior->setVisible(true);
}

void ElemPlate::calcMatrixInternal()
{
    _mt.assign(1, lengthSI() / ior(), 0, 1);
    _ms = _mt;
}

void ElemPlate::setSubRangeSI(double value)
{
    _mt1.assign(1, value / ior(), 0, 1/ior());
    _ms1 = _mt1;
    _mt2.assign(1, lengthSI() - value, 0, ior());
    _ms2 = _mt2;
}

//------------------------------------------------------------------------------
//                              ElemCurveMirror

ElemCurveMirror::ElemCurveMirror()
{
    _radius = new Z::Parameter(Z::Dims::linear(), QStringLiteral("R"), QStringLiteral("R"),
                               qApp->translate("Param", "Radius of curvature"),
                               qApp->translate("Param", "Positive for concave mirror, negative for convex mirror."));
    _alpha = new Z::Parameter(Z::Dims::angular(), QStringLiteral("Alpha"), Z::Strs::alpha(),
                              qApp->translate("Param", "Angle of incidence "),
                              qApp->translate("Param", "Zero angle is normal incidence."));
    addParam(_radius, 100, Z::Units::mm());
    addParam(_alpha, 0, Z::Units::deg());

    _radius->setVerifier(globalCurvatureRadiusVerifier());
}

void ElemCurveMirror::calcMatrixInternal()
{
    _mt.assign(1, 0, -2.0 / radius() / cos(alpha()), 1);
    _ms.assign(1, 0, -2.0 / radius() * cos(alpha()), 1);
}

//------------------------------------------------------------------------------
//                                ElemThinLens

ElemThinLens::ElemThinLens()
{
    _focus = new Z::Parameter(Z::Dims::linear(), QStringLiteral("F"), QStringLiteral("F"),
                               qApp->translate("Param", "Focal length"),
                               qApp->translate("Param", "Positive for collecting lens, negative for diverging lens."));
    _alpha = new Z::Parameter(Z::Dims::angular(), QStringLiteral("Alpha"), Z::Strs::alpha(),
                              qApp->translate("Param", "Angle of incidence "),
                              qApp->translate("Param", "Zero angle is normal incidence."));
    addParam(_focus, 100, Z::Units::mm());
    addParam(_alpha, 0, Z::Units::deg());

    _focus->setVerifier(globalFocalLengthVerifier());
}

void ElemThinLens::calcMatrixInternal()
{
    _mt.assign(1.0, 0.0, -1.0 / focus() / cos(alpha()), 1.0);
    _ms.assign(1.0, 0.0, -1.0 / focus() * cos(alpha()), 1.0);
}

//------------------------------------------------------------------------------
//                              ElemCylinderLensT

void ElemCylinderLensT::calcMatrixInternal()
{
    _mt.assign(1.0, 0.0, -1.0 / focus() / cos(alpha()), 1.0);
    _ms.unity();
}

//------------------------------------------------------------------------------
//                              ElemCylinderLensS

void ElemCylinderLensS::calcMatrixInternal()
{
    _mt.unity();
    _ms.assign(1.0, 0.0, -1.0 / focus() * cos(alpha()), 1.0);
}

//------------------------------------------------------------------------------
//                             ElemTiltedCrystal

ElemTiltedCrystal::ElemTiltedCrystal() : ElementRange()
{
    _ior->setVisible(true);

    _alpha = new Z::Parameter(Z::Dims::angular(), QStringLiteral("Alpha"), Z::Strs::alpha(),
                              qApp->translate("Param", "Angle of incidence "),
                              qApp->translate("Param", "Zero angle is normal incidence."));
    addParam(_alpha, 0, Z::Units::deg());
}

void ElemTiltedCrystal::calcMatrixInternal()
{
    double L = lengthSI();
    double a = alpha();
    double n = ior();

    _mt.assign(1, L * n * SQR(cos(a)) / (SQR(n) - SQR(sin(a))), 0.0, 1.0);
    _ms.assign(1, L / n, 0, 1);
}

void ElemTiltedCrystal::setSubRangeSI(double value)
{
    double n = ior();
    double cos_a = cos(alpha());
    double cos_b = cos(asin(sin(alpha()) / n)); // cosine of angle inside medium
    double cos_ab = cos_a / cos_b;
    double cos_ba = cos_b / cos_a;
    double L1 = value;
    double L2 = lengthSI() - value;

    //  --> /:: -->  half lengh * input to medium
    _mt1.assign(cos_ba, L1/n * cos_ab, 0, 1/n * cos_ab);
    _ms1.assign(1, L1/n, 0, 1/n);

    //  --> ::/ -->  output from media * half length
    _mt2.assign(cos_ab, L2 * cos_ab, 0, n * cos_ba);
    _ms2.assign(1, L2, 0, n);
}

//------------------------------------------------------------------------------
//                              ElemTiltedPlate

void ElemTiltedPlate::calcMatrixInternal()
{
    double L = lengthSI();
    double n = ior();
    double sin_a = sin(alpha());
    double s = n*n - sin_a*sin_a;

    _mt.assign(1, L * n*n * (1 - sin_a*sin_a) / sqrt(s*s*s), 0, 1);
    _ms.assign(1, L / sqrt(s), 0, 1);
}

void ElemTiltedPlate::setSubRangeSI(double value)
{
    double n = ior();
    double cos_a = cos(alpha());
    double cos_b = cos(asin( sin(alpha()) / n)); // cosine of angle inside medium
    double cos_ab = cos_a / cos_b;
    double cos_ba = cos_b / cos_a;
    double L1 = value;
    double L2 = axisLengthSI() - L1;

    //  --> /:: -->  half lengh * input to medium
    _mt1.assign(cos_ba, L1/n * cos_ab, 0, 1/n * cos_ab);
    _ms1.assign(1, L1/n, 0, 1/n);

    //  --> ::/ -->  output from medium * half length
    _mt2.assign(cos_ab, L2 * cos_ab, 0, n * cos_ba);
    _ms2.assign(1, L2, 0, n);
}

double ElemTiltedPlate::axisLengthSI() const
{
    return lengthSI() / cos( asin( sin( alpha() ) / ior() ) );
}

//------------------------------------------------------------------------------
//                            ElemBrewsterCrystal

ElemBrewsterCrystal::ElemBrewsterCrystal() : ElementRange()
{
    _ior->setVisible(true);
}

void ElemBrewsterCrystal::calcMatrixInternal()
{
    double L = lengthSI();
    double n = ior();

    _ms.assign(1, L / n, 0, 1);
    _mt.assign(1, _ms.B / SQR(n), 0, 1);
}

void ElemBrewsterCrystal::setSubRangeSI(double value)
{
    double n = ior();
    double L1 = value;
    double L2 = lengthSI() - L1;

    //  --> /:: -->  half lengh * input to medium
    _mt1.assign(n, L1/n/n, 0, 1/n/n);
    _ms1.assign(1, L1/n,   0, 1/n);

    //  --> ::/ -->  output from media * half length
    _mt2.assign(1/n, L2/n, 0, n*n);
    _ms2.assign(1, L2, 0, n);
}

//------------------------------------------------------------------------------
//                             ElemBrewsterPlate

ElemBrewsterPlate::ElemBrewsterPlate() : ElementRange()
{
    _ior->setVisible(true);
}

void ElemBrewsterPlate::calcMatrixInternal()
{
    double n = ior();

    _ms.assign(1, axisLengthSI() / n, 0, 1);
    _mt.assign(1, _ms.B / SQR(n), 0, 1);
}

void ElemBrewsterPlate::setSubRangeSI(double value)
{
    double L1 = value;
    double L2 = axisLengthSI() - L1;
    double n = ior();

    //  --> /:: -->  half lengh * input to medium
    _mt1.assign(n, L1/n/n, 0, 1/n/n);
    _ms1.assign(1, L1/n, 0, 1/n);

    //  --> ::/ -->  output from media * half length
    _mt2.assign(1/n, L2/n, 0, n*n);
    _ms2.assign(1, L2, 0, n);
}

double ElemBrewsterPlate::axisLengthSI() const
{
    // L_eff = L / cos( asin( sin( atan(n) )/n ) ) = L * Sqrt(n^2 + 1) / n
    double n = ior();
    return lengthSI() * sqrt(n*n + 1) / n;
}

//------------------------------------------------------------------------------
//                                ElemMatrix

void ElemMatrix::makeParam(const QString& name, const double& value)
{
    addParam(new Z::Parameter(Z::Dims::none(), name, name, name), value, Z::Units::none());
}

ElemMatrix::ElemMatrix()
{
    makeParam("At", 1);
    makeParam("Bt", 0);
    makeParam("Ct", 0);
    makeParam("Dt", 1);

    makeParam("As", 1);
    makeParam("Bs", 0);
    makeParam("Cs", 0);
    makeParam("Ds", 1);
}

void ElemMatrix::setMatrix(int offset, const double& a, const double& b, const double& c, const double& d)
{
    params().at(offset+0)->setValue(Z::Value(a, Z::Units::none()));
    params().at(offset+1)->setValue(Z::Value(b, Z::Units::none()));
    params().at(offset+2)->setValue(Z::Value(c, Z::Units::none()));
    params().at(offset+3)->setValue(Z::Value(d, Z::Units::none()));
}

// TODO:NEXT-VER checkParameter(): can A and D be 0 and what does it mean?

void ElemMatrix::calcMatrixInternal()
{
    _mt.assign(_params.at(0)->value().value(), _params.at(1)->value().value(),
               _params.at(2)->value().value(), _params.at(3)->value().value());
    _ms.assign(_params.at(4)->value().value(), _params.at(5)->value().value(),
               _params.at(6)->value().value(), _params.at(7)->value().value());
}

//------------------------------------------------------------------------------
//                             ElemNormalInterface

void ElemNormalInterface::calcMatrixInternal()
{
    const double n1 = ior1();
    const double n2 = ior2();

    _mt.assign(1, 0, 0, n1 / n2);
    _ms = _mt;

    _mt_inv.assign(1, 0, 0, n2 / n1);
    _ms_inv = _mt_inv;
}
