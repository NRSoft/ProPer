#include "remotefile.h"

using namespace ProPer;

RemoteFile::RemoteFile(QObject *parent) : QObject(parent)
{
    _logger = spdlog::get("main");
    _logger->debug("creating new remote file");
}


///////  s e t  U r l  ///////
void RemoteFile::setUrl(const QString& path, const QString& username, const QString& password)
{
    _logger->debug("setting remote file URL to \"{}\"", path.toUtf8().data());

    _url_path = path;
    _user_name = username;
    _password = password;

    _url.clear();
    _url = QUrl::fromEncoded(path.toLatin1());
    _url.setUserName(username);
    _url.setPassword(password);
}


//////  d o w n l o a d  ///////
void RemoteFile::download()
{
    _logger->debug("start downloading");
    disconnect(&_manager, SIGNAL(finished(QNetworkReply*)), 0, 0);
    connect(&_manager, SIGNAL(finished(QNetworkReply*)), SLOT(downloadFinished(QNetworkReply*)));

    QNetworkRequest request(_url);
    QNetworkReply* reply = _manager.get(request);

//#ifndef QT_NO_SSL
//    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
//#endif
}


///////  d o w n l o a d  F i n i s h e d  ///////
void RemoteFile::downloadFinished(QNetworkReply *reply)
{
    if(reply->error()){
        _data.clear();
        _logger->error("download failed: {}", reply->errorString().toUtf8().data());
    }
    else{
        _data = reply->readAll();
        _logger->debug("download succeeded");
    }
    reply->deleteLater();
    emit fileDownloaded();
}


///////  u p l o a d  //////
void RemoteFile::upload(const QByteArray& data)
{
    if(data.isEmpty())
        return;

    _logger->debug("start uploading");
    disconnect(&_manager, SIGNAL(finished(QNetworkReply*)), 0, 0);
    connect(&_manager, SIGNAL(finished(QNetworkReply*)), SLOT(uploadFinished(QNetworkReply*)));

    QNetworkRequest request(_url);
    _manager.put(request, data);

}


//////  u p l o a d  F i n i s h e d  ///////
void RemoteFile::uploadFinished(QNetworkReply *reply)
{
    if(reply->error())
        _logger->error("upload failed: {}", reply->errorString().toUtf8().data());
    else
        _logger->debug("upload succeeded");

    reply->deleteLater();
    emit fileUploaded();
}

//void RemoteFile::sslErrors(const QList<QSslError> &sslErrors)
//{
//#ifndef QT_NO_SSL
//    foreach (const QSslError &error, sslErrors)
//        _logger->error(SSL error: {}", error.errorString().toUtf8().data());
//#else
//    Q_UNUSED(sslErrors);
//#endif
//}

