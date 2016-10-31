#include <QString>
#include <QShortcut>
#include <QMessageBox>
#include <QColorDialog>

#include "hyperlinkdialog.h"
#include "categorydialog.h"
#include "taskdialog.h"
#include "ui_taskdialog.h"

using namespace ProPer;

TaskDialog::TaskDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TaskDialog)
{
    _logger = spdlog::get("user");
    _logger->debug("creating task dialog");

    ui->setupUi(this);
//    setTabOrder(ui->nameEdit, ui->descriptionEdit);
//    setTabOrder(ui->categoriesCombo, ui->categoriesButton);
//    setTabOrder(ui->categoriesButton, ui->statusCombo);
//    setTabOrder(ui->statusCombo, ui->deadlineDate);
//    setTabOrder(ui->deadlineDate, ui->buttonBox);
//    ui->deadlineDate->setDate(QDate::currentDate().addDays(28)); // deafult is four weeks away

    _noteEditing = false;
    ui->notesEdit->setProperty("readOnly", true);
    ui->notesEdit->setOpenExternalLinks(true);

    QShortcut *boldShortcut;
    boldShortcut = new QShortcut(Qt::CTRL + Qt::Key_B, this);
    connect(boldShortcut, SIGNAL(activated()), this, SLOT(ctrlB_clicked()));

    QShortcut *italicShortcut;
    italicShortcut = new QShortcut(Qt::CTRL + Qt::Key_I, this);
    connect(italicShortcut, SIGNAL(activated()), this, SLOT(ctrlI_clicked()));

    QShortcut *linkShortcut;
    linkShortcut = new QShortcut(Qt::CTRL + Qt::Key_K, this);
    connect(linkShortcut, SIGNAL(activated()), this, SLOT(ctrlK_clicked()));

    _type = TASK;
    _currentNote = -1;
    _updateNote();

    ui->nameEdit->setPlaceholderText(QLatin1String("Enter name here"));
}


TaskDialog::~TaskDialog()
{
    _logger->debug("exiting task dialog");
    delete ui;
}


//////  i n i t  N e w  ///////
bool TaskDialog::initNew(QTreeWidget *tree, Task* parent)
{
    _logger->debug("initialising new task dialog");
    if(tree == nullptr){
        _logger->warn("cannot run TaskDialog with no tree defined");
        return false;
    }

    _tree = tree;
    _task.clear();

    _type = parent? TASK: PROJECT; // no children by definition
    ui->nameLabel->setText(parent? QLatin1String("Task"): QLatin1String("Project"));
    _task.setStatus(Task::BACKLOG);

    _initCommon();

     return true;
}

////////  i n i t  E d i t  ///////
bool TaskDialog::initEdit(QTreeWidget* tree, Task* task)
{
    _logger->debug("initialising task editing dialog");
    if(tree == nullptr || task == nullptr){
        _logger->warn("cannot run TaskDialog with no tree defined ({}) or task specified ({})",
                      tree == nullptr, task == nullptr);
        return false;
    }

    _tree = tree;
    _task = *task;

    _type = task->isProject()? PROJECT: task->isTask()? TASK: COMPONENT;
    ui->nameLabel->setText(_type==PROJECT? QLatin1String("Project"):
                              _type==TASK? QLatin1String("Task"):
                                           QLatin1String("Component"));

    _initCommon();

    return true;
}


////////  i n i t  C o m m o n  ///////
void TaskDialog::_initCommon()
{
    _updateCategories();

    ui->nameEdit->setText(_task.name());
    ui->descriptionEdit->setPlainText(_task.description());
    ui->statusCombo->setCurrentText(_task.statusString());

    ui->deadlineDate->setDate(_task.deadline());
    ui->deadlineDate->setEnabled(_type == TASK); // deadlines only for the lower-level tasks

    _currentNote = _task.noteCount()>0? 0: -1;
    _updateNote();

    ui->nameEdit->setFocus();
}


////////  s e t  T a s k  ///////
void TaskDialog::setTask(const Task *task)
{
    ui->nameEdit->setText(task->name());
}


/////  o n  b u t t o n  B o x  a c c e p t e d  /////
void TaskDialog::on_buttonBox_accepted()
{
    _task.setName(ui->nameEdit->text());
    _task.setDescription(ui->descriptionEdit->toPlainText());
    _task.assignStatus(ui->statusCombo->currentText());
    _task.assignCategory(ui->categoriesCombo->currentText());
   //TODO: check parent deadlines, warn the user and adjust them accordingly
    _task.setDeadline(ui->deadlineDate->date());
    _logger->debug("Task \"{}\" accepted", _task.name().toStdString());
}


///////  o n  p r e v  N o t e  B u t t o n  c l i c k e d  ///////
void TaskDialog::on_prevNoteButton_clicked()
{
    if(!_noteEditing && _currentNote > 0){
        _currentNote--;
        _updateNote();
    }
}


///////  o n  n e x t  N o t e  B u t t o n  c l i c k e d  ///////
void TaskDialog::on_nextNoteButton_clicked()
{
    if(!_noteEditing && _currentNote < _task.noteCount()-1){
        _currentNote++;
        _updateNote();
    }
}


/////  o n  n e w  N o t e  B u t t o n  c l i c k e d  /////
void TaskDialog::on_newNoteButton_clicked()
{
    if(_noteEditing) return;

    Note emptyNote;
    _task.appendNote(emptyNote);
    _currentNote = _task.noteCount() - 1;
    _updateNote();

    on_editNoteButton_clicked(); // start editing
}


/////  o n  e d i t  N o t e  B u t t o n  c l i c k e d  /////
void TaskDialog::on_editNoteButton_clicked()
{
    if(_noteEditing){ // save note
        if(ui->notesEdit->toPlainText().isEmpty())
            on_deleteNoteButton_clicked(); // empty notes are not allowed
        else{
            Note note;
            note.setDateTime(QDateTime::currentDateTime());
            note.setHtml(ui->notesEdit->toHtml());
            note.pruneHtml();
            _task.updateNote(_currentNote, note);
        }
        _noteEditing = false;
        _updateNote();
    }
    else{ // start editing note
        ui->prevNoteButton->setEnabled(false);
        ui->nextNoteButton->setEnabled(false);
        ui->notesEdit->setPlaceholderText(QLatin1String("Enter text here, apply Ctrl-B, Ctrl-I or Ctrl-K to selections"));
        ui->notesEdit->setFocus();
        _noteEditing = true;
    }

    ui->newNoteButton->setEnabled(!_noteEditing);
    ui->editNoteButton->setText(_noteEditing? QLatin1String("Save"): QLatin1String("Edit"));
    ui->deleteNoteButton->setText(_noteEditing? QLatin1String("Cancel"): QLatin1String("Delete"));
    ui->notesEdit->setProperty("readOnly", !_noteEditing);
}


/////  o n  d e l e t e  N o t e  B u t t o n  c l i c k e d  /////
void TaskDialog::on_deleteNoteButton_clicked() // also cancel editing
{
    if(_currentNote < 0) return;

    QString title = _noteEditing? QLatin1String("Cancel editing"): QLatin1String("Delete note");
    QString text  = _noteEditing? QLatin1String("Discard changes?"): QLatin1String("Delete current note?");
    if(QMessageBox::warning(this, title, text, QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes)
        return;

    if(_noteEditing){ // cancel
        _noteEditing = false;
        ui->newNoteButton->setEnabled(true);
        ui->editNoteButton->setText(QLatin1String("Edit"));
        ui->deleteNoteButton->setText(QLatin1String("Delete"));
        ui->notesEdit->setProperty("readOnly", true);
    }
    else{ // delete
        _task.deleteNote(_currentNote);
        if(_currentNote >= _task.noteCount())
            _currentNote--;
    }
    _updateNote();
}


/////  c a t e g o r i e s  B u t t o n :  c l i c k e d  ///////
void TaskDialog::on_categoriesButton_clicked()
{
    CategoryDialog dlg(_tree, this);
    dlg.setWindowFlags(dlg.windowFlags()&(~Qt::WindowContextHelpButtonHint)); // no questions!
    if(dlg.exec()){
        _updateCategories();
    }
}


/////  o n  d e a d l i n e  D a t e  C h a n g e d  /////
void TaskDialog::on_deadlineDate_dateChanged(const QDate &date)
{
    qint64 shift = QDate::currentDate().daysTo(date);
    if(shift < 0){
        ui->deadlineDate->setDate(QDate::currentDate());
        shift = 0;
    }

    QString urgency("(finish in ");
    if(shift == 0) urgency = QLatin1String("(finish today)");
    else if(shift == 1) urgency = QLatin1String("(finish tomorrow)");
    else if(shift < 21) urgency += QString::number(shift) + QStringLiteral(" days)");
    else if(shift < 90) urgency += QString::number((shift+1)/7) + QStringLiteral(" weeks)"); // rounded
    else if(shift < 365) urgency += QString::number((shift+10)/31) + QStringLiteral(" months)");
    else urgency += QString::number(shift/365.0, 'f', 1) + QStringLiteral(" years)");
    ui->urgencyLabel->setText(urgency);
}


//////  c t r l - B  c l i c k e d  //////
void TaskDialog::ctrlB_clicked()
{
    if(ui->notesEdit == focusWidget() && ui->notesEdit->textCursor().hasSelection() && _noteEditing){
        QTextCharFormat format = ui->notesEdit->textCursor().charFormat();
        if(format.fontWeight() == QFont::Bold)
            format.setFontWeight(QFont::Normal);
        else
            format.setFontWeight(QFont::Bold);
        ui->notesEdit->textCursor().mergeCharFormat(format);
    }
}


//////  c t r l - I  c l i c k e d  //////
void TaskDialog::ctrlI_clicked()
{
    if(ui->notesEdit == focusWidget() && ui->notesEdit->textCursor().hasSelection() && _noteEditing){
        QTextCharFormat format = ui->notesEdit->textCursor().charFormat();
        if(format.fontItalic())
            format.setFontItalic(false);
        else
            format.setFontItalic(true);
        ui->notesEdit->textCursor().mergeCharFormat(format);
    }
}


//////  c t r l - K  c l i c k e d  //////
void TaskDialog::ctrlK_clicked()
{
    if(ui->notesEdit == focusWidget() && ui->notesEdit->textCursor().hasSelection() && _noteEditing){
        QTextCursor cursor = ui->notesEdit->textCursor();
        QTextCharFormat format = cursor.charFormat();
        if(format.isAnchor()){
            format.setAnchor(false);
            format.setFontUnderline(false);
            format.setForeground(QBrush(Qt::black));
            cursor.mergeCharFormat(format);
        }
        else{
            HyperlinkDialog d;
            d.setName(cursor.selectedText());
            if(d.exec()){
                format.setAnchor(true);
                format.setFontUnderline(true);
                format.setForeground(QBrush(Qt::blue));
                format.setAnchorName(d.name());
                format.setAnchorHref(d.link());
                cursor.insertText(d.name(), format);
            }
        }
    }
}


/////////  u p d a t e  N o t e  ////////
void TaskDialog::_updateNote()
{
    if(_currentNote >= 0){
        ui->noteNumberLabel->setText(QLatin1String("(") + QString::number(_currentNote+1) +
                                     QStringLiteral(" of ") + QString::number(_task.noteCount()) + QStringLiteral(")"));
        ui->noteDateLabel->setText(_task.note(_currentNote).date().toString(QLatin1String("d/MM/yy")));
        ui->noteTimeLabel->setText(_task.note(_currentNote).time().toString(QLatin1String("H:mm:ss")));
        ui->notesEdit->setHtml(_task.note(_currentNote).html());
        ui->prevNoteButton->setEnabled(_currentNote > 0);
        ui->nextNoteButton->setEnabled(_currentNote < _task.noteCount()-1);
    }
    else{ // no notes in the task
        ui->noteNumberLabel->clear();
        ui->noteDateLabel->clear();
        ui->noteTimeLabel->clear();
        ui->notesEdit->clear();
        ui->prevNoteButton->setEnabled(false);
        ui->nextNoteButton->setEnabled(false);
        ui->newNoteButton->setEnabled(true);
    }
    ui->editNoteButton->setEnabled(_currentNote >= 0);
    ui->deleteNoteButton->setEnabled(_currentNote >= 0);
}


/////////  u p d a t e  C a t e g o r i e s  ////////
void TaskDialog::_updateCategories()
{
    ui->categoriesCombo->clear();
    QHash<QString,QColor>::const_iterator cat;
    for(cat=Task::categories().cbegin(); cat!=Task::categories().cend(); ++cat)
        ui->categoriesCombo->addItem(cat.key());
    ui->categoriesCombo->setCurrentText(_task.category());
    ui->categoriesCombo->model()->sort(0); // alphabetically
}

