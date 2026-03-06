#ifndef DOCKPANEL_H
#define DOCKPANEL_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class DockPanel;
}
QT_END_NAMESPACE

class QTreeView;

class DockPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DockPanel(QWidget *parent = nullptr);
    ~DockPanel();

    QTreeView *treeView() const;

private:
    Ui::DockPanel *ui;
};

#endif // DOCKPANEL_H
