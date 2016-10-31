#pragma once

#include <QObject>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include "spdlog/spdlog.h"

namespace ProPer {

class RemoteFile : public QObject
{
    Q_OBJECT
public:
    explicit RemoteFile(QObject *parent = 0);

    void setUrl(const QString& path, const QString& username, const QString& password);

    inline const QString& getUrlPath() const {return _url_path;}
    inline QString getUsername() const {return _user_name;}
    inline const QString& getPassword() const {return _password;}

    void download();
    inline const QByteArray& getData() const {return _data;} // after downloading

    void upload(const QByteArray& data);

signals:
    void fileDownloaded();
    void fileUploaded();

public slots:
    void downloadFinished(QNetworkReply* reply);
    void uploadFinished(QNetworkReply* reply);
//    void sslErrors(const QList<QSslError>& errors);

private:
    QUrl _url;
    QString _url_path;
    QString _user_name;
    QString _password;

    QByteArray _data;

    QNetworkAccessManager _manager;

    std::shared_ptr<spdlog::logger> _logger;
};

} // namespace;
