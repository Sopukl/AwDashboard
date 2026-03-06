#include "MainWindow.h"
#include "DockPanel.h"
#include "./ui_mainwindow.h"
#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeView>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->actionSubWindowView->setChecked(true);
    ui->actionTabbedView->setChecked(false);
    dockPanel = new DockPanel(this);
    ui->dockWidget->setWidget(dockPanel);
    treeModel = new QStandardItemModel(this);
    treeModel->setHorizontalHeaderLabels(QStringList() << "Nodes");
    dockPanel->treeView()->setModel(treeModel);
    dockPanel->treeView()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(dockPanel->treeView(), &QTreeView::customContextMenuRequested, this, &MainWindow::showTreeContextMenu);
    auto *viewMenu = menuBar()->addMenu("View");
    auto *toggleDockAction = ui->dockWidget->toggleViewAction();
    toggleDockAction->setText("Показать/скрыть dock widget");
    viewMenu->addAction(toggleDockAction);
    QSettings settings;
    restoreGeometry(settings.value("mainWindow/geometry").toByteArray());
    restoreState(settings.value("mainWindow/state").toByteArray());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("mainWindow/geometry", saveGeometry());
    settings.setValue("mainWindow/state", saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::on_actionNew_triggered()
{
    treeModel->clear();
    treeModel->setHorizontalHeaderLabels(QStringList() << "Nodes");
    nodeCounter = 0;
    currentTreeFilePath.clear();
}

void MainWindow::on_actionOpen_triggered()
{
    const QString filePath = QFileDialog::getOpenFileName(this, "Open XML", QString(), "XML Files (*.xml)");
    if (filePath.isEmpty()) {
        return;
    }
    if (!loadTreeFromFile(filePath)) {
        QMessageBox::warning(this, "Open XML", "Cannot load XML file.");
    }
}

void MainWindow::on_actionSave_triggered()
{
    QString filePath = currentTreeFilePath;
    if (filePath.isEmpty()) {
        filePath = QFileDialog::getSaveFileName(this, "Save XML", QString(), "XML Files (*.xml)");
        if (filePath.isEmpty()) {
            return;
        }
    }
    if (!saveTreeToFile(filePath)) {
        QMessageBox::warning(this, "Save XML", "Cannot save XML file.");
        return;
    }
    currentTreeFilePath = filePath;
}

void MainWindow::on_actionClose_triggered()
{
    treeModel->clear();
    treeModel->setHorizontalHeaderLabels(QStringList() << "Nodes");
    nodeCounter = 0;
    currentTreeFilePath.clear();
}

void MainWindow::on_actionSubWindowView_triggered()
{
    ui->mdiArea->setViewMode(QMdiArea::SubWindowView);
    ui->actionSubWindowView->setChecked(true);
    ui->actionTabbedView->setChecked(false);
}

void MainWindow::on_actionTabbedView_triggered()
{
    ui->mdiArea->setViewMode(QMdiArea::TabbedView);
    ui->actionSubWindowView->setChecked(false);
    ui->actionTabbedView->setChecked(true);
}

void MainWindow::showTreeContextMenu(const QPoint &pos)
{
    contextIndex = dockPanel->treeView()->indexAt(pos);
    QMenu menu(this);
    auto *addNodeAction = menu.addAction("Add node");
    connect(addNodeAction, &QAction::triggered, this, &MainWindow::addNode);
    menu.exec(dockPanel->treeView()->viewport()->mapToGlobal(pos));
}

void MainWindow::addNode()
{
    const QStringList nodeTypes = {"Item", "Text", "Rectangle"};
    bool ok = false;
    const QString selectedType = QInputDialog::getItem(this, "Add node", "Node type:", nodeTypes, 0, false, &ok);
    if (!ok || selectedType.isEmpty()) {
        return;
    }
    auto *item = new QStandardItem(QString("%1 %2").arg(selectedType).arg(++nodeCounter));
    if (contextIndex.isValid()) {
        auto *parentItem = treeModel->itemFromIndex(contextIndex);
        parentItem->appendRow(item);
        dockPanel->treeView()->expand(contextIndex);
        return;
    }
    treeModel->appendRow(item);
}

void MainWindow::appendItemToXml(QXmlStreamWriter &writer, QStandardItem *item) const
{
    writer.writeStartElement("node");
    writer.writeAttribute("text", item->text());
    for (int i = 0; i < item->rowCount(); ++i) {
        appendItemToXml(writer, item->child(i));
    }
    writer.writeEndElement();
}

void MainWindow::loadItemFromXml(QXmlStreamReader &reader, QStandardItem *parentItem)
{
    const QString nodeText = reader.attributes().value("text").toString();
    auto *item = new QStandardItem(nodeText);
    parentItem->appendRow(item);
    const QRegularExpression re("(\\d+)$");
    const QRegularExpressionMatch match = re.match(nodeText);
    if (match.hasMatch()) {
        const int value = match.captured(1).toInt();
        if (value > nodeCounter) {
            nodeCounter = value;
        }
    }
    while (!(reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == "node") && !reader.atEnd()) {
        reader.readNext();
        if (reader.tokenType() == QXmlStreamReader::StartElement && reader.name() == "node") {
            loadItemFromXml(reader, item);
        }
    }
}

bool MainWindow::saveTreeToFile(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }
    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("tree");
    for (int i = 0; i < treeModel->rowCount(); ++i) {
        appendItemToXml(writer, treeModel->item(i));
    }
    writer.writeEndElement();
    writer.writeEndDocument();
    return !writer.hasError();
}

bool MainWindow::loadTreeFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QXmlStreamReader reader(&file);
    treeModel->clear();
    treeModel->setHorizontalHeaderLabels(QStringList() << "Nodes");
    nodeCounter = 0;
    bool rootFound = false;
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.tokenType() == QXmlStreamReader::StartElement && reader.name() == "tree") {
            rootFound = true;
            while (!(reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == "tree") && !reader.atEnd()) {
                reader.readNext();
                if (reader.tokenType() == QXmlStreamReader::StartElement && reader.name() == "node") {
                    loadItemFromXml(reader, treeModel->invisibleRootItem());
                }
            }
        }
    }
    if (!rootFound || reader.hasError()) {
        treeModel->clear();
        treeModel->setHorizontalHeaderLabels(QStringList() << "Nodes");
        nodeCounter = 0;
        currentTreeFilePath.clear();
        return false;
    }
    currentTreeFilePath = filePath;
    return true;
}
