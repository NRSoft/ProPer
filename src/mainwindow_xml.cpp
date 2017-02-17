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
                             QStringLiteral(".") + QString::number(VERSION_MINOR));
    xmlWriter.writeStartElement("projects");
    for (int i = 0; i < ui->projectTree->topLevelItemCount(); ++i){
        Task* item = dynamic_cast<Task*>(ui->projectTree->topLevelItem(i));
        item->saveToXml(xmlWriter);
    }
    xmlWriter.writeEndElement(); // projects
    Task::saveCategoriesToXml(xmlWriter);
    xmlWriter.writeStartElement("history");
    for (int i = 0; i < ui->listWidget->count(); ++i)
        xmlWriter.writeTextElement("record", ui->listWidget->item(i)->text());
    xmlWriter.writeEndElement(); // history
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
            else if(xmlReader.name() == QLatin1String("categories")){
                if(!Task::loadCategoriesFromXml(xmlReader))
                    _mainLogger->error("cannot read XML file");
            }
            else if(xmlReader.name() == QLatin1String("history")){
                ui->listWidget->clear();
                for(;;){
                    xmlReader.readNext();
                    if(xmlReader.isStartElement()){
                        if(xmlReader.name() == QLatin1String("record"))
                            ui->listWidget->addItem(xmlReader.readElementText());
                    }
                    else if(xmlReader.isEndElement()){
                        if(xmlReader.name() == QLatin1String("history"))
                            break;
                    }
                }
            } // "history"
        }
    }
}


///////  s a v e  T o  L o c a l  X m l  ////////
bool MainWindow::_saveToLocalXml(QString& filename)
{
    if(filename.isEmpty())
    {
        QString name = QFileDialog::getSaveFileName(this,
                                    QLatin1String("Save Projects"), ".",
                                    QLatin1String("Project Files (*.xml)"));
        if(name.isEmpty()) return false;
        filename = name;
    }

    _mainLogger->info("saving data to \"{}\"", filename.toUtf8().data());

    QFile file(filename);
    if(!file.open(QFile::WriteOnly | QFile::Text)){
        _mainLogger->error("cannot open file \"{}\"", filename.toUtf8().data());
        return false;
    }

    QByteArray data;
    _saveToByteArray(&data);
    file.write(data);

    file.close();
    if(file.error()){
        _mainLogger->error("cannot write to file \"{}\"", filename.toUtf8().data());
        return false;
    }

    return true;
}


///////  l o a d  F r o m  L o c a l  X m l  //////
bool MainWindow::_loadFromLocalXml(QString& filename)
{
    if(filename.isEmpty())
    {
        QString name = QFileDialog::getOpenFileName(this,
                                    QLatin1String("Open Project File"), ".",
                                    QLatin1String("Project Files (*.xml)"));
        if(name.isEmpty()) return false;
        filename = name;
    }

    _mainLogger->info("loading data from \"{}\"", filename.toUtf8().data());

    QFile file(filename);
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        _mainLogger->error("cannot open file \"{%s}}\"", filename.toUtf8().data());
        return false;
    }

    _loadFromByteArray(file.readAll());

    file.close();
    if(file.error()){
        _mainLogger->error("cannot read file \"{}\"", filename.toUtf8().data());
        return false;
    }

    _mainLogger->info("finished loading data from \"{}\"", filename.toUtf8().data());

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
        _mainLogger->debug("uploading to the remote file \"{}\"", _remote_file.getUrlPath().toUtf8().data());
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
        _mainLogger->debug("downloading from the remote file \"{}\"", _remote_file.getUrlPath().toUtf8().data());
        _remote_file.download();
    }
}
