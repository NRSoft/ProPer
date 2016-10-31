#include <QFile>
#include <QFileDialog>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "task.h"
#include "remotefile.h"
#include "remoteaccessdialog.h"

#include "version.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace ProPer;


/////  s a v e  T o  B y t e  A r r a y  //////
void MainWindow::_saveToByteArray(QByteArray* data)
{
    _mainLogger->debug("saving data to byte array");

    QXmlStreamWriter xmlWriter(data);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("ProPer");
    xmlWriter.writeAttribute(QStringLiteral("version"), QString::number(VERSION_MAJOR) +\
                             QStringLiteral(".") + QString::number(VERSION_MINOR) +\
                             QStringLiteral(".") + QString::number(VERSION_BUILD));
    xmlWriter.writeStartElement("projects");
    for (int i = 0; i < ui->projectTree->topLevelItemCount(); ++i){
        Task* item = dynamic_cast<Task*>(ui->projectTree->topLevelItem(i));
        item->saveToXml(xmlWriter);
    }
    xmlWriter.writeEndElement(); // projects
    Task::saveCategoriesToXml(xmlWriter);
    xmlWriter.writeEndElement(); // ProPer
    xmlWriter.writeEndDocument();
    Task::setModified(false);
}


/////  l o a d  F r o m  B y t e  A r r a y  /////
void MainWindow::_loadFromByteArray(const QByteArray& data)
{
    _mainLogger->debug("loading data from byte array");

    Task::setModified(false);

    ui->projectTree->clear();

    QXmlStreamReader xmlReader(data);
    while(!xmlReader.atEnd()){
        if(xmlReader.readNext() == QXmlStreamReader::Invalid){
            _mainLogger->error("ill-formated XML data");
            break;
        }
        if(xmlReader.isStartElement()){
            if(xmlReader.name() == QLatin1String("project")){
                Task* item = new Task();
                item->loadFromXml(xmlReader);
                ui->projectTree->addTopLevelItem(item);
            }
            else if(xmlReader.name() == QLatin1String("categories"))
                if(!Task::loadCategoriesFromXml(xmlReader))
                    _mainLogger->error("cannot read XML file");

        }
    }
}


///////  s a v e  T o  L o c a l  X m l  ////////
bool MainWindow::_saveToLocalXml(const QString* filename)
{
    QString name = (filename)? *filename: QFileDialog::getSaveFileName(this,
                                          QLatin1String("Save Projects"), "", QLatin1String("Project Files (*.xml)"));
    if(name.isEmpty()) return false;

    _mainLogger->info("saving data to \"{}\"", name.toStdString());

    QFile file(name);
    if(!file.open(QFile::WriteOnly | QFile::Text)){
        _mainLogger->error("cannot open file \"{}\"", name.toStdString());
        return false;
    }

    QByteArray data;
    _saveToByteArray(&data);
    file.write(data);

    file.close();
    if(file.error()){
        _mainLogger->error("cannot write to file \"{}\"", name.toStdString());
        return false;
    }
    _localFileName = name;
    return true;
}


///////  l o a d  F r o m  L o c a l  X m l  //////
bool MainWindow::_loadFromLocalXml(const QString* filename)
{
    QString name = (filename)? *filename: QFileDialog::getOpenFileName(this,
                                          QLatin1String("Open Project File"), ".", QLatin1String("Project Files (*.xml)"));
    if(name.isEmpty()) return false;

    _mainLogger->info("loading data from \"{}\"", name.toStdString());

    QFile file(name);
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        _mainLogger->error("cannot open file \"{}\"", name.toStdString());
        return false;
    }

    _loadFromByteArray(file.readAll());

    file.close();
    if(file.error()){
        _mainLogger->error("cannot read file \"{}\"", name.toStdString());
        return false;
    }
    _localFileName = name;
    return true;
}


///////  s a v e  T o  R e m o t e  X m l  ///////
void MainWindow::_saveToRemoteXml()
{
    RemoteAccessDialog dialog(this);
    dialog.readConfigFrom(_remote_file);
    if(dialog.exec()){
        QApplication::setOverrideCursor(Qt::WaitCursor);
        dialog.wrtieConfigTo(_remote_file);
        _mainLogger->debug("uploading to the remote file \"{}\"", _remote_file.getUrlPath().toStdString());
        QByteArray data;
        _saveToByteArray(&data);
        _remote_file.upload(data);
    }
}


/////  l o a d  F r o m  R e m o t e  X m l  /////
void MainWindow::_loadFromRemoteXml()
{
    RemoteAccessDialog dialog(this);
    dialog.readConfigFrom(_remote_file);
    if(dialog.exec()){
        QApplication::setOverrideCursor(Qt::WaitCursor);
        dialog.wrtieConfigTo(_remote_file);
        _mainLogger->debug("downloading from the remote file \"{}\"", _remote_file.getUrlPath().toStdString());
        _remote_file.download();
    }
}
