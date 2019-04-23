#ifndef Z_HELP_SYSTEM_H
#define Z_HELP_SYSTEM_H

#include <QObject>

namespace Z {

class HelpSystem : public QObject
{
    Q_OBJECT

public:
    static HelpSystem* instance();

    void setParent(QWidget* parent) { _parent = parent; }

public slots:
    void showContents();
    void showIndex();
    void visitHomePage();
    void sendBugReport();
    void checkUpdates();
    void showAbout();

private:
    HelpSystem();

    QWidget* _parent = nullptr;
};

namespace Help {

void show(const QString& topic);

} // namespace Help
} // namespace Z

#endif // Z_HELP_SYSTEM_H
