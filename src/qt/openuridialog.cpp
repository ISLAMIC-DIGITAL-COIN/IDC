// Copyright (c) 2019 The PIVX developers
// Copyright (c) 2021 The ISLAMIC DIGITAL COIN developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "openuridialog.h"
#include "ui_openuridialog.h"

#include "guiutil.h"
#include "qt/islamic_digital_coin/qtutils.h"
#include "walletmodel.h"

#include <QFile>
#include <QUrl>

OpenURIDialog::OpenURIDialog(QWidget* parent) : QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
                                                ui(new Ui::OpenURIDialog)
{
    ui->setupUi(this);
    this->setStyleSheet(parent->styleSheet());
    ui->uriEdit->setPlaceholderText("ISLAMIC_DIGITAL_COIN:");

    ui->labelSubtitle->setText("URI");
    setCssProperty(ui->labelSubtitle, "text-title2-dialog");
    setCssProperty(ui->frame, "container-dialog");
    setCssProperty(ui->labelTitle, "text-title-dialog");

    setCssBtnPrimary(ui->pushButtonOK);
    setCssBtnPrimary(ui->selectFileButton);
    setCssProperty(ui->pushButtonCancel, "btn-dialog-cancel");

    initCssEditLine(ui->uriEdit, true);
    connect(ui->pushButtonOK, &QPushButton::clicked, this, &OpenURIDialog::accept);
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &OpenURIDialog::close);
}

void OpenURIDialog::showEvent(QShowEvent* event)
{
    ui->uriEdit->setFocus();
}

OpenURIDialog::~OpenURIDialog()
{
    delete ui;
}

QString OpenURIDialog::getURI()
{
    return ui->uriEdit->text();
}

void OpenURIDialog::accept()
{
    SendCoinsRecipient rcp;
    if (GUIUtil::parseBitcoinURI(getURI(), &rcp)) {
        /* Only accept value URIs */
        QDialog::accept();
    } else {
        setCssEditLineDialog(ui->uriEdit, false, true);
    }
}

void OpenURIDialog::on_selectFileButton_clicked()
{
    QString filename = GUIUtil::getOpenFileName(this, tr("Select payment request file to open"), "", "", NULL);
    if (filename.isEmpty())
        return;

    QFile file(filename);
    if (!file.exists()) {
        inform(tr("File not found"));
        return;
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray r = file.readAll();
        if (r.size() > 200) {
            inform(tr("Parsed data too large"));
            return;
        }

        QString str = QString::fromStdString(std::string(r.constData(), r.length()));
        if (!str.startsWith("islamic digital coin")) {
            inform(tr("Invalid URI, not starting with \"islamic digital coin\" prefix"));
        }
        ui->uriEdit->setText(str);
    }
}

void OpenURIDialog::inform(const QString& str)
{
    if (!snackBar) snackBar = new SnackBar(nullptr, this);
    snackBar->setText(str);
    snackBar->resize(this->width(), snackBar->height());
    openDialog(snackBar, this);
}