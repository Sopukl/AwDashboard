#include "DockPanel.h"
#include "./ui_dockpanel.h"

DockPanel::DockPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DockPanel)
{
    ui->setupUi(this);
}

DockPanel::~DockPanel()
{
    delete ui;
}

QTreeView *DockPanel::treeView() const
{
    return ui->treeView;
}
