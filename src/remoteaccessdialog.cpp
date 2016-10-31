#include "remoteaccessdialog.h"
#include "ui_remoteaccessdialog.h"

RemoteAccessDialog::RemoteAccessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoteAccessDialog)
{
    ui->setupUi(this);
    ui->passwordEdit->setEchoMode(QLineEdit::Password); // mask password with stars
}


RemoteAccessDialog::~RemoteAccessDialog()
{
    delete ui;
}


void RemoteAccessDialog::readConfigFrom(const ProPer::RemoteFile& remote_file)
{
    ui->urlEdit->setText(remote_file.getUrlPath());
    ui->usernameEdit->setText(remote_file.getUsername());
    ui->passwordEdit->setText(remote_file.getPassword());
}


void RemoteAccessDialog::wrtieConfigTo(ProPer::RemoteFile& remote_file) const
{
    remote_file.setUrl(ui->urlEdit->text(), ui->usernameEdit->text(), ui->passwordEdit->text());
}
