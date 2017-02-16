#pragma once

#include <memory>

#include <QMainWindow>
#include <QTreeWidget>
#include <QGraphicsScene>

#include "spdlog/spdlog.h"

#include "projectheaderview.h"
#include "remotefile.h"
#include "taskpipe.h"

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void _saveToByteArray(QByteArray* data);
    void _loadFromByteArray(const QByteArray& data);
    bool _saveToLocalXml(QString& filename);
    bool _loadFromLocalXml(QString& filename);
    void _saveToRemoteXml();
    void _loadFromRemoteXml();
    void _saveSettings();
    void _loadSettings();

    void _hideCompleted();

private slots:

    void on_actionNewProject_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionExit_triggered();

    void on_projectTree_customContextMenuRequested(const QPoint &pos);
    void on_projectTree_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_newTask();
    void on_editTask();
    void on_completeTask();
    void on_deleteTask();

    void on_actionOpen_Remote_triggered();

    void on_actionSave_Remote_triggered();

    void on_remoteFileDownloaded();
    void on_remoteFileUploaded();

    void on_actionSave_As_triggered();

    void on_actionHide_Completed_triggered();

    void _showTime();

    void on_actionNew_triggered();

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
    ProjectHeaderView* _header;

    TaskPipe* _pipe;

    QString _settingsFile;
    QString _localFileName;
    ProPer::RemoteFile _remote_file;

    std::shared_ptr<spdlog::logger> _mainLogger;
    std::shared_ptr<spdlog::logger> _taskLogger;
    std::shared_ptr<spdlog::logger> _userLogger;
};
