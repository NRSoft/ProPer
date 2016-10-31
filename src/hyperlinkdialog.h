#pragma once

#include <QString>
#include <QDialog>

namespace Ui {
class HyperlinkDialog;
}

class HyperlinkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HyperlinkDialog(QWidget *parent = 0);
    ~HyperlinkDialog();

    const QString name();
    void setName(const QString& name);

    const QString link(); // always has "http://" in the beginning (auto generated)
    void setLink(const QString& link); // will strip "http://" from the beginning for display

private:
    Ui::HyperlinkDialog *ui;
};
