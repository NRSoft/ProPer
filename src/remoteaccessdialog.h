#pragma once

#include <QDialog>
#include "remotefile.h"

namespace Ui {
class RemoteAccessDialog;
}

class RemoteAccessDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteAccessDialog(QWidget *parent = 0);
    ~RemoteAccessDialog();

    void readConfigFrom(const ProPer::RemoteFile& remote_file);
    void wrtieConfigTo(ProPer::RemoteFile& remote_file) const;

private slots:

private:
    Ui::RemoteAccessDialog *ui;
};
