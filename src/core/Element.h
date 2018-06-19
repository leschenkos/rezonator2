#ifndef ELEMENT_H
#define ELEMENT_H

#include "Math.h"
#include "Parameters.h"
#include "core/OriTemplates.h"

#define DECLARE_ELEMENT(class_name, base_class)\
    class class_name : public base_class\
    {\
    protected:\
        Element* create() const override { return new class_name(); }\
    public:\
        const QString type() const override { return QStringLiteral(# class_name); }\
        static const QString _type_() { return QStringLiteral(# class_name); }


#define DECLARE_ELEMENT_END };

#define DEFAULT_LABEL(label)\
    const QString labelPrefix() const override { return QStringLiteral(label); }

#define TYPE_NAME(name)\
    const QString typeName() const override { static QString _name_ = name; return _name_; }

#define PARAMS_EDITOR(editor)\
    Z::ParamsEditor paramsEditorKind() override { return Z::ParamsEditor::editor; }

#define CALC_MATRIX\
    void calcMatrixInternal() override;

#define SUB_RANGE\
    void setSubRangeSI(double value) override;

#define CHECK_PARAM\
    const char* checkParameter(Z::Parameter *param, double newValue) const;

#define AXIS_LEN\
    double axisLengthSI() const override;

class Element;

//------------------------------------------------------------------------------
/**
    Base class for objects who wish to own optical elements.
*/
class ElementOwner
{
public:
    virtual void elementChanged(Element*) {}
    virtual int indexOf(Element*) const { return -1; }
    virtual int count() const { return 0; }
    friend class Element;
};

//------------------------------------------------------------------------------
/**
    Base class for all optical elements.
*/
class Element : public Z::ParameterListener
{
public:
    virtual ~Element();

    ElementOwner* owner() const { return _owner; }
    void setOwner(ElementOwner *owner);

    int id() const { return _id; }

    /// Function returns type of element, e.g. "FlatMirror". Type is used for
    /// internal identification of element class like true class name.
    virtual const QString type() const = 0;

    /// Function returns "human-friendly" name of element type, e.g. "Flat mirror".
    virtual const QString typeName() const { return type(); }

    /// Default prefix for generating of automatical labels for elements of this type.
    virtual const QString labelPrefix() const { return QString(); }

    Z::Parameters& params() { return _params; }
    bool hasParams() const { return !_params.isEmpty(); }

    /// Label of element. Label is short indentificator
    /// for element or its name (like variable name). E.g.: "M1", "L_f", etc.
    const QString& label() const { return _label; }
    void setLabel(const QString& value);

    /// User title of element.
    /// E.g.: "Output coupler", "Folding mirror", etc.
    const QString& title() const { return _title; }
    void setTitle(const QString& value);

    /// Returns element label, or element index if the label is empty.
    QString displayLabel();

    /// Returns element title, or displayLabel() if the title is empty.
    QString displayTitle();

    /// Returns element title and label as "{elem_label} ({elem_title})".
    /// Or only one of thoses if the other is empty, or number and type
    /// of element if both label and title are empty.
    QString displayLabelTitle();

    void calcMatrix();
    const Z::Matrix& Mt() const { return _mt; }
    const Z::Matrix& Ms() const { return _ms; }
    const Z::Matrix* pMt() const { return &_mt; }
    const Z::Matrix* pMs() const { return &_ms; }

    /// Preferable parameter editor kind for this element.
    virtual Z::ParamsEditor paramsEditorKind() { return Z::ParamsEditor::List; }

    bool disabled() const { return _disabled; }
    void setDisabled(bool value);

    /// Locks element. Locking pervents element from generating 'modified' event
    /// and matrix recalculation every time when parameter value has been changed.
    void lock();

    /// Unlocks element. Calculates new matrix. No events generated here.
    void unlock(bool recalc = true);

protected:
    Element();

    Z::Parameters _params;
    ElementOwner* _owner = nullptr; ///< Pointer to an object who owns this element.
    bool _disabled = false;
    bool _locked = false;
    QString _label, _title;
    Z::Matrix _mt, _ms;
    int _id;

    virtual void calcMatrixInternal();

    int addParam(Z::Parameter* param, const double& value, Z::Unit unit);

    void parameterChanged(Z::ParameterBase*) override;

    /// Support for ElementsCatalog's functionality
    friend class ElementsCatalog;
    virtual Element* create() const = 0;
};

typedef QVector<Element*> Elements;

//------------------------------------------------------------------------------
/**
    The base class for elements having length and optional IOR.
*/
class ElementRange : public Element
{
public:
    virtual void setSubRangeSI(double value) { Q_UNUSED(value) }

    const Z::Matrix& Mt1() const { return _mt1; }
    const Z::Matrix& Ms1() const { return _ms1; }
    const Z::Matrix& Mt2() const { return _mt2; }
    const Z::Matrix& Ms2() const { return _ms2; }
    const Z::Matrix* pMt1() const { return &_mt1; }
    const Z::Matrix* pMs1() const { return &_ms1; }
    const Z::Matrix* pMt2() const { return &_mt2; }
    const Z::Matrix* pMs2() const { return &_ms2; }

    Z::Parameter* paramLength() const { return _length; }
    Z::Parameter* paramIor() const { return _ior; }
    double lengthSI() const { return _length->value().toSi(); }
    double ior() const { return _ior->value().value(); }

    virtual double axisLengthSI() const { return lengthSI(); }
    virtual double opticalPathSI() const { return axisLengthSI()* ior(); }

protected:
    ElementRange();

    Z::Matrix _mt1, _mt2;
    Z::Matrix _ms1, _ms2;
    Z::Parameter *_length;
    Z::Parameter *_ior;
};

//------------------------------------------------------------------------------
/**
    The base class for elements representing an interface between two media.
    An interface element is characterized by two IORs - `ior1` and `ior2`.
    Where `ior1` is IOR of a medium at 'the left' of the interface (medium 1),
    and `ior2` is IOR of a medium at 'the right' of the interface (medium 2).
    Element has two set of matrices - IN and OUT matrices.
    Default matrices are taken when beam passes from the medium 1 to the medium 2,
    inverted matrices are taken when beam passes from the medium 2 to the medium 1.
    Which set of matrices should be taken is defined by @a RoundTripCalculator.
*/
class ElementInterface : public Element
{
public:
    const Z::Matrix& Mt_inv() const { return _mt_inv; }
    const Z::Matrix& Ms_inv() const { return _ms_inv; }
    const Z::Matrix* pMt_inv() const { return &_mt_inv; }
    const Z::Matrix* pMs_inv() const { return &_ms_inv; }

    Z::Parameter* paramIor1() const { return _ior1; }
    Z::Parameter* paramIor2() const { return _ior2; }

    double ior1() const { return _ior1->value().value(); }
    double ior2() const { return _ior2->value().value(); }

protected:
    ElementInterface();

    Z::Matrix _mt_inv, _ms_inv;
    Z::Parameter *_ior1, *_ior2;
};

//------------------------------------------------------------------------------
class ElementLocker
{
public:
    ElementLocker(Element* elem, bool recalc = true): _elem(elem), _recalc(recalc)
    {
        _elem->lock();
    }
    ~ElementLocker()
    {
        _elem->unlock(_recalc);
    }
private:
    Element *_elem;
    bool _recalc;
};

//------------------------------------------------------------------------------
/**
    Generator of automatical labels for elements.
    Automatical label consist of a prefix like 'M', 'L', etc. and index.
*/
class ElementsNamer : public Singleton<ElementsNamer>
{
public:
    QString generateLabel(const QString& prefix);
    void reset();

private:
    QMap<QString, int> _autoLabels;

    friend class Singleton<ElementsNamer>;
};


//------------------------------------------------------------------------------
//                                Z::Utils

namespace Z {
namespace Utils {

void generateLabel(Element *elem);
inline bool isRange(Element *elem) { return dynamic_cast<ElementRange*>(elem); }
inline ElementRange* asRange(Element *elem) { return dynamic_cast<ElementRange*>(elem); }

/// Gives a filter of parameters for regular users' usage.
ParameterFilter* defaultParamFilter();

} // namespace Utils
} // namespace Z

#endif // ELEMENT_H
