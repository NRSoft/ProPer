#include <QTime>
#include <QTimer>
#include <QMessageBox>
#include <QComboBox>
#include <QSettings>

#include "version.h"
#include "task.h"
#include "taskdialog.h"
#include "taskbubble.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace ProPer;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    _mainLogger = spdlog::get("main");
    _taskLogger = spdlog::get("task");
    _userLogger = spdlog::get("user");
    _userLogger->debug("creating main window");

    ui->setupUi(this);
    setWindowTitle(QLatin1String("ProPer Project Management"));

    QComboBox* combo = new QComboBox(ui->mainToolBar);
    ui->mainToolBar->addWidget(combo);
    ui->mainToolBar->setMovable(false);

    //Task::categories()

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(_showTime()));
    timer->start(1000);
    _showTime();

    //_hiddenIfCompleted = false;

    _settingsFile = QApplication::applicationDirPath() + QStringLiteral("/ProPer.ini");
    _loadSettings();

    if(!_localFileName.isEmpty())
        _loadFromLocalXml(_localFileName);

    connect(&_remote_file, SIGNAL(fileDownloaded()), this, SLOT(on_remoteFileDownloaded()));
    connect(&_remote_file, SIGNAL(fileUploaded()), this, SLOT(on_remoteFileUploaded()));

    ui->splitter->setStretchFactor(1, 1); // make graphics window more stretched

    ui->projectTree->setSortingEnabled(false);
    ui->projectTree->setColumnCount(1);
    ui->projectTree->setHeaderLabel("Projects");

    _header = new ProjectHeaderView(ui->projectTree);
    _header->setHistoryList(ui->listWidget);
    ui->projectTree->setHeader(_header);
    ui->projectTree->header()->setSectionsClickable(true);

    ui->projectTree->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->projectTree->setDragEnabled(true);
    ui->projectTree->viewport()->setAcceptDrops(true);
    ui->projectTree->setDropIndicatorShown(true);
    ui->projectTree->setDragDropMode(QAbstractItemView::InternalMove);

    ui->projectTree->setContextMenuPolicy(Qt::CustomContextMenu);

    _pipe = new TaskPipe(this);
    ui->graphicsView->setScene(_pipe);
    _pipe->setSceneRect(-300, -150, 600, 300);//ui->graphicsView->sceneRect());
    _pipe->build(ui->projectTree);
}


MainWindow::~MainWindow()
{
    if(Task::isModified()){
        QMessageBox::StandardButton reply;
        QString message = QLatin1String("Tasks have been modified, save?");
        reply = QMessageBox::question(this, QLatin1String("Data Update"), message, QMessageBox::Yes|QMessageBox::No);
        if(reply == QMessageBox::Yes)
            _saveToLocalXml(_localFileName);
    }

    _saveSettings();

    _userLogger->debug("destroying main window");
    delete ui;
}

void MainWindow::on_actionNew_triggered()
{
    _taskLogger->debug("erasing projects and tasks");
    ui->projectTree->clear();
    TaskBubble::setDistanceOrigin(0);
    _pipe->build(ui->projectTree);
    _localFileName.clear();
}

void MainWindow::on_actionNewProject_triggered()
{
    _userLogger->debug("command: add new project");
    ui->projectTree->setCurrentItem(nullptr);
    on_newTask();
}

void MainWindow::on_actionOpen_triggered()
{
    _userLogger->debug("command: open data file");
    QString name;
    if(_loadFromLocalXml(name))
        _localFileName = name;
    TaskBubble::setDistanceOrigin(0);
    _pipe->build(ui->projectTree);
}

void MainWindow::on_actionSave_triggered()
{
    _userLogger->debug("command: save data file");
    _saveToLocalXml(_localFileName);
}

void MainWindow::on_actionSave_As_triggered()
{
    _userLogger->debug("command: save data file as");
    QString name;
    if(_saveToLocalXml(name))
        _localFileName = name;
}

void MainWindow::on_actionOpen_Remote_triggered()
{
    _userLogger->debug("command: open remote file");
    _loadFromRemoteXml();
    TaskBubble::setDistanceOrigin(0);
    _pipe->build(ui->projectTree);
}

void MainWindow::on_actionSave_Remote_triggered()
{
    _userLogger->debug("command: save to remote file");
    _saveToRemoteXml();
}

void MainWindow::on_actionExit_triggered()
{
    _userLogger->debug("command: exit");
    QApplication::quit();
}


void MainWindow::on_actionHide_Completed_triggered()
{
    _userLogger->debug("command: hide completed");
    Task::setHiddenIfCompleted(!Task::hiddenIfCompleted());
    ui->actionHide_Completed->setText(Task::hiddenIfCompleted()? QLatin1String("Show Completed"):
                                                                 QLatin1String("Hide Completed"));
    _hideCompleted();
}


void MainWindow::on_projectTree_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    on_editTask();

    Q_UNUSED(item);
    Q_UNUSED(column);
}


void MainWindow::on_projectTree_customContextMenuRequested(const QPoint &pos)
{
    Task* item = dynamic_cast<Task*>(ui->projectTree->itemAt(pos));
    if(item == nullptr)
        ui->projectTree->setCurrentItem(nullptr);

    QMenu *menu=new QMenu(this);
    if(item){
        menu->addAction(QLatin1String("New Task"), this, SLOT(on_newTask()));

        if(item->isProject())
            menu->addAction(QLatin1String("Edit Project"), this, SLOT(on_editTask()));
        else if(item->isTask())
            menu->addAction(QLatin1String("Edit Task"), this, SLOT(on_editTask()));
        else
            menu->addAction(QLatin1String("Edit Component"), this, SLOT(on_editTask()));

        menu->addSeparator();
        if(item->isTask() && item->status() != Task::DONE)
            menu->addAction(QLatin1String("Mark Complete"), this, SLOT(on_completeTask()));

        if(item->isProject())
            menu->addAction(QLatin1String("Delete Project"), this, SLOT(on_deleteTask()));
        else if(item->isTask())
            menu->addAction(QLatin1String("Delete Task"), this, SLOT(on_deleteTask()));
        else
            menu->addAction(QLatin1String("Delete Component"), this, SLOT(on_deleteTask()));
    }
    else{
        menu->addAction(QLatin1String("New Project"), this, SLOT(on_newTask()));
    }
    menu->popup(ui->projectTree->viewport()->mapToGlobal(pos));
}


void MainWindow::on_newTask()
{
    _userLogger->debug("command: new task");
    Task* parent = dynamic_cast<Task*>(ui->projectTree->currentItem());

    if(!parent){ // top-level project, reuse the code
        ProjectHeaderView* header = static_cast<ProjectHeaderView*>(ui->projectTree->header());
        header->on_newProject();
    }
    else{ // task under selected one
        Task* task = new Task();
        parent->addChild(task);
        task->setText(0, QLatin1String("[editing...]"));
        if(task->runEditor()){
            _pipe->build(ui->projectTree);
            _taskLogger->debug("new task \"{}\"", task->name().toUtf8().data());
            ui->listWidget->insertItem(0, task->fullPrefix() + QStringLiteral(" was created"));
        }
        else // if cancelled remove newly added item
            parent->takeChild(parent->indexOfChild(task));
    }
}


void MainWindow::on_editTask()
{
    _userLogger->debug("command: edit task");
    Task* task = dynamic_cast<Task*>(ui->projectTree->currentItem());
    if(task == nullptr) return;
    QString originalName = task->name();

    if(task->runEditor()){
        if(task->name() != originalName)
            ui->listWidget->insertItem(0, task->fullPrefix(false) + originalName +
                    QStringLiteral("\" was renamed to \"") + task->name() + QStringLiteral("\""));
        else
            ui->listWidget->insertItem(0, task->fullPrefix() + QStringLiteral(" was updated"));
        _taskLogger->debug("updated task \"{}\"", task->name().toUtf8().data());
        _pipe->build(ui->projectTree);
    }
}


void MainWindow::on_completeTask()
{
    Task* task = dynamic_cast<Task*>(ui->projectTree->currentItem());
    task->setStatus(Task::DONE);
    task->hideCompleted();
    ui->listWidget->insertItem(0, task->fullPrefix() + QStringLiteral(" was completed"));
    _taskLogger->debug("completed task \"{}\"", task->name().toUtf8().data());
    Task::setModified(true);
}


void MainWindow::on_deleteTask()
{
    _userLogger->debug("command: delete task");
    QTreeWidget* tree = ui->projectTree;
    Task* task = dynamic_cast<Task*>(tree->currentItem());
    QString message = QLatin1String("Delete ") + (task->isProject()? QStringLiteral("project \""): QStringLiteral("task \""))
                                               +  task->text(0) + QStringLiteral("\"?");
    QMessageBox::StandardButton reply = QMessageBox::question(this, QLatin1String("Delete"), message, QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes){
        ui->listWidget->insertItem(0, task->fullPrefix() + QStringLiteral(" was deleted"));
        _taskLogger->debug("deleting task \"{}\"", task->name().toUtf8().data());
        Task* p = dynamic_cast<Task*>(task->parent());
        if(p)
            p->takeChild(p->indexOfChild(task));
        else
            tree->takeTopLevelItem(tree->indexOfTopLevelItem(task));
        delete task;
        Task::setModified(true);
    }
}


void MainWindow::on_remoteFileDownloaded()
{
    QApplication::restoreOverrideCursor();
    const QByteArray& data = _remote_file.getData();
    _mainLogger->info("downloaded project tree from remote file \"{0}\", total size = {1}",
                      _remote_file.getUrlPath().toUtf8().data(), data.size());
    if(!data.isEmpty())
        _loadFromByteArray(data);
}


void MainWindow::on_remoteFileUploaded()
{
    _mainLogger->info("uploaded project tree to remote file \"{}\"",
                      _remote_file.getUrlPath().toUtf8().data());
    QApplication::restoreOverrideCursor();
}


void MainWindow::on_actionAbout_triggered()
{
    QString text(" ProPer v ");
    text += QString::number(VERSION_MAJOR) + QStringLiteral(".") + QString::number(VERSION_MINOR);
    text += QStringLiteral("  (C) NRSoft 2016\nhttps://github.com/NRSoft/ProPer");
    QMessageBox::about(this, QLatin1String("About"), text);
}


void MainWindow::_saveSettings()
{
    _mainLogger->info("saving settings to \"{}\"", _settingsFile.toUtf8().data());
    QSettings settings(_settingsFile, QSettings::IniFormat);
    settings.setValue(QLatin1String("project_file"), _localFileName);
    settings.setValue(QLatin1String("remote_file"), _remote_file.getUrlPath());
    settings.setValue(QLatin1String("user_name"), _remote_file.getUsername());
}


void MainWindow::_loadSettings()
{
    _mainLogger->info("loading settings from \"{}\"", _settingsFile.toUtf8().data());
    QSettings settings(_settingsFile, QSettings::IniFormat);
    _localFileName = settings.value(QLatin1String("project_file")).toString();
    _remote_file.setUrl(settings.value(QLatin1String("remote_file")).toString(),
                        settings.value(QLatin1String("user_name")).toString(), QString());
}

void MainWindow::_hideCompleted()
{
     for(int i=0; i<ui->projectTree->topLevelItemCount(); ++i){
        Task* project = dynamic_cast<Task*>(ui->projectTree->topLevelItem(i));
        project->hideCompleted(); // all tasks recursively
    }
}


void MainWindow::_showTime()
{
    QDateTime date_time = QDateTime::currentDateTime();
    QString text = date_time.toString(QLatin1String("ddd dd MMM  HH:mm"));
    if ((date_time.time().second() % 2) == 0){
        text[text.indexOf(':')] = ' ';
    }
    ui->clockLabel->setText(text);
}
