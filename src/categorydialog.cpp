#include <QColorDialog>
#include "categorydialog.h"
#include "ui_categorydialog.h"

using namespace ProPer;


CategoryDialog::CategoryDialog(QTreeWidget *tree, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CategoryDialog)
{
    _logger = spdlog::get("user");
    _logger->debug("creating category dialog");

    ui->setupUi(this);

    _tree = tree;

    _localCategories = Task::categories();

    if(_localCategories.isEmpty()) return;

    QHash<QString,QColor>::const_iterator cat;
    for(cat=_localCategories.cbegin(); cat!=_localCategories.cend(); ++cat)
        ui->categoryCombo->addItem(cat.key());
    ui->categoryCombo->model()->sort(0); // alphabetically
}


CategoryDialog::~CategoryDialog()
{
    _logger->debug("exiting category dialog");
    delete ui;
}


//////  b u t t o n  B o x :  a c c e p t e d  //////
void CategoryDialog::on_buttonBox_accepted()
{
    QHash<QString,QColor>::const_iterator cat = Task::categories().begin();

    // remove deleted categories
    while(cat!=Task::categories().end())
        if(!_localCategories.contains(cat.key()))
            Task::categories().erase(cat);
        else
            ++cat;

    // reassign colors, adding new categories as it goes
    for(cat=_localCategories.cbegin(); cat!=_localCategories.cend(); ++cat)
        Task::setCategoryColor(cat.key(), cat.value());

    _logger->debug("category list updates accepted");
}


//////  c o l o r  B u t t o n :  c l i c k e d  ///////
void CategoryDialog::on_colorButton_clicked()
{
    QColorDialog dlg;
    if(dlg.exec()){
        _currentColor = dlg.selectedColor();
        QString s = "background-color: " + _currentColor.name() + ";";
        ui->colorButton->setStyleSheet(s);
        if(_localCategories.contains(ui->categoryCombo->currentText()))
            _localCategories[ui->categoryCombo->currentText()] = _currentColor;
    }
}


///////  a d d  B u t t o n :  c l i c k e d  ///////
void CategoryDialog::on_addButton_clicked()
{
    const QString& category = ui->categoryCombo->currentText();
    if(_localCategories.contains(category)) return;

    _localCategories[category] = _currentColor;

    ui->categoryCombo->addItem(category);
    ui->categoryCombo->model()->sort(0); // alphabetically
    _taskCount(category);
    _logger->debug("category \"{}\" added", category.toStdString());
}


///////  d e l e t e  B u t t o n :  c l i c k e d  //////
void CategoryDialog::on_deleteButton_clicked()
{
    _logger->debug("category \"{}\" to be deleted", ui->categoryCombo->currentText().toStdString());
    _localCategories.remove(ui->categoryCombo->currentText());
    ui->categoryCombo->removeItem(ui->categoryCombo->currentIndex());
}


/////  c a t e g o r y  C o m b o :  c u r r e n t  T e x t  C h a n g e d   //////
void CategoryDialog::on_categoryCombo_currentTextChanged(const QString &arg1)
{
    if(_localCategories.contains(arg1)){
        _currentColor = _localCategories.value(arg1);
        QString s = "background-color: " + _currentColor.name() + ";";
        ui->colorButton->setStyleSheet(s);
        _taskCount(arg1);
    }
}


////////  t a s k  C o u n t  ///////
size_t CategoryDialog::_taskCount(const QString& category)
{
    size_t count = 0;
    for(int i=0; i<_tree->topLevelItemCount(); ++i){
        const Task* project = dynamic_cast<const Task*>(_tree->topLevelItem(i));
        if(project->category() == category)
            count++;
        count += _taskCountPerItem(project, category);
    }
    if(count)
        ui->usedLabel->setText(QString("used in ") + QString::number(count) + QStringLiteral(" tasks"));
    else
        ui->usedLabel->setText(QLatin1String("not used"));
    ui->deleteButton->setEnabled(count==0);
    return count;
}


////////  t a s k  C o u n t  P e r  I t e m  ///////
size_t CategoryDialog::_taskCountPerItem(const Task* task, const QString& category)
{
    size_t count = 0;
    for(int i=0; i<task->childCount(); ++i){
        const Task* t = dynamic_cast<const Task*>(task->child(i));
        if(t->category() == category)
            count++;
        count += _taskCountPerItem(t, category);
    }
    return count;
}
