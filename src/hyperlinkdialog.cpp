#include "hyperlinkdialog.h"
#include "ui_hyperlinkdialog.h"

HyperlinkDialog::HyperlinkDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HyperlinkDialog)
{
    ui->setupUi(this);
    setTabOrder(ui->nameEdit, ui->linkEdit);
    ui->linkEdit->setPlaceholderText(QLatin1String("external link (starting with http:// or https://)"));
    ui->nameEdit->setFocus();
}

HyperlinkDialog::~HyperlinkDialog()
{
    delete ui;
}


const QString HyperlinkDialog::name()
{
    return ui->nameEdit->text();
}


void HyperlinkDialog::setName(const QString& name)
{
    ui->nameEdit->setText(name);
}


const QString HyperlinkDialog::link()
{
    QString hyperlink = ui->linkEdit->text();
    if(hyperlink.left(7) == QLatin1String("http://") ||
            hyperlink.left(8) == QLatin1String("https://"))
        return hyperlink;

    QString str = QLatin1String("http://") + hyperlink; // forcefully adding
    return str;
}


void HyperlinkDialog::setLink(const QString& link)
{
    if(link.left(7) == QLatin1String("http://"))
        ui->linkEdit->setText(link.mid(7));
    else
        ui->linkEdit->setText(link);
}
