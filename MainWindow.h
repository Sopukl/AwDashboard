#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QModelIndex>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionClose_triggered();
    void on_actionSubWindowView_triggered();
    void on_actionTabbedView_triggered();
    void showTreeContextMenu(const QPoint &pos);
    void addNode();

private:
    void appendItemToXml(class QXmlStreamWriter &writer, class QStandardItem *item) const;
    void loadItemFromXml(class QXmlStreamReader &reader, class QStandardItem *parentItem);
    bool saveTreeToFile(const QString &filePath) const;
    bool loadTreeFromFile(const QString &filePath);
    Ui::MainWindow *ui;
    class DockPanel *dockPanel = nullptr;
    QModelIndex contextIndex;
    int nodeCounter = 0;
    class QStandardItemModel *treeModel = nullptr;
    QString currentTreeFilePath;
};
#endif // MAINWINDOW_H
