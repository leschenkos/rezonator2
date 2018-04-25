#ifndef PLOT_FUNC_WINDOW_H
#define PLOT_FUNC_WINDOW_H

#include <QToolButton>

#include "../SchemaWindows.h"
#include "../funcs/PlotFunction.h"

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QSplitter;
QT_END_NAMESPACE

class QCPCursor;
class QCPGraph;
typedef QCPGraph Graph;

namespace Ori {
namespace Widgets {
//class ImagedLabel;
class StatusBar;
}}

class CursorPanel;
class FrozenStateButton;
class Plot;
class PlotFunction;
class PlotFuncWindow;
class PlotParamsPanel;
class SchemaStorable;

//------------------------------------------------------------------------------

class FunctionModeButton : public QToolButton
{
    Q_OBJECT

public:
    FunctionModeButton(const QString& icon, const QString& text, int mode);
    int mode() const { return _mode; }

private:
    int _mode;
};

//------------------------------------------------------------------------------

class FuncOptionsPanel : public QWidget
{
    Q_OBJECT

public:
    FuncOptionsPanel(PlotFuncWindow *window);

protected:
    QMap<int, FunctionModeButton*> _modeButtons;

    virtual int currentFunctionMode() const { return 0; }
    virtual void functionModeChanged(int mode) { Q_UNUSED(mode); }
    void showCurrentMode();

    QWidget* makeSectionHeader(const QString& title);
    QWidget* makeModeButton(const QString& icon, const QString& text, int mode);

private:
    PlotFuncWindow *_window;
    void modeButtonClicked();
};

//------------------------------------------------------------------------------

class PlotFuncWindow : public SchemaMdiChild
{
    Q_OBJECT

public:
    explicit PlotFuncWindow(PlotFunction*);
    ~PlotFuncWindow();

    PlotFunction* function() const { return _function; }
    Plot* plot() const { return _plot; }

    /// Do autolimits after next update.
    void requestAutolimits() { _autolimitsRequest = true; }

    /// Cursor should be ceneterd after next update.
    void requestCenterCursor() { _centerCursorRequested = true; }

    /// Edits function parameters through dialog.
    bool configure(QWidget* parent);

    // inherits from BasicMdiChild
    QList<QMenu*> menus() override { return QList<QMenu*>() << menuPlot << menuLimits /* TODO:NEXT-VER << menuFormat*/; }

    void schemaChanged(Schema*) override;

    void storeView(int key);
    void restoreView(int key);

public slots:
    void update();

protected slots:
    virtual void updateNotables();
    virtual void updateDataGrid();

protected:
    PlotFunction* _function;
    Plot* _plot;

    QMenu *menuPlot, *menuLimits, *menuFormat;

    /// Calculates function and plots its results.
    virtual void calculate(bool replot = true);

    virtual bool configureInternal(QWidget* parent) { Q_UNUSED(parent); return true; }

    bool _needRecalc = false, _frozen = false;
    QVector<Graph*> _graphsT, _graphsS;
    PlotParamsPanel* _leftPanel;
    QCPCursor* _cursor;
    CursorPanel* _cursorPanel;
    QSplitter* _splitter;
    Ori::Widgets::StatusBar* _statusBar;
    FrozenStateButton* _buttonFrozenInfo;
    bool _autolimitsRequest = false; ///< If autolimits requested after next update.
    bool _centerCursorRequested = false; ///< If cursor should be centered after next update.

    QAction *actnShowT, *actnShowS, *actnShowTS,
        *actnAutolimits, *actnAutolimitsX, *actnAutolimitsY,
        *actnSetLimits, *actnSetLimitsX, *actnSetLimitsY,
        *actnZoomIn, *actnZoomOut, *actnZoomInX, *actnZoomOutX, *actnZoomInY, *actnZoomOutY,
        *actnUpdate, *actnUpdateParams, *actnShowRoundTrip, *actnFreeze, *actnFrozenInfo;

    Graph* selectedGraph() const;

    void createActions();
    void createMenuBar();
    void createToolBar();
    void createStatusBar();
    void createContent();

    void updateVisibilityTS();
    void updateTSModeActions();
    void updateAxesTitles();
    void updateGraphs(Z::WorkPlane);

    void showInfo(const QString& text, const QString& icon = QString());

    QPen getLineSettings(Z::WorkPlane);

    void debug_LogGraphsCount();

    virtual QWidget* makeOptionsPanel() { return nullptr; }

private slots:
    void showT();
    void showS();
    void showTS();
    void updateWithParams();
    void graphSelected(Graph*);
    void updateCursorInfo();
    void showRoundTrip();
    void freeze(bool);

    QWidget* optionsPanelRequired();

private:
    struct ViewState
    {
        QPair<double, double> limitsX;
        QPair<double, double> limitsY;
        QPointF cursorPos;
    };
    QMap<int, ViewState> _storedView;

    const double _zoomStep;
};

#endif // PLOT_FUNC_WINDOW_H
