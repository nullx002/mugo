/*
    mugo, sgf editor.
    Copyright (C) 2009-2010 nsase.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <QDebug>
#include <QSettings>
#include <QTextCodec>
#include "newdocumentdialog.h"
#include "ui_newdocumentdialog.h"
#include "mugoapp.h"

NewDocumentDialog::NewDocumentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewDocumentDialog)
{
    ui->setupUi(this);

    // get board settings.
    QSettings settings;
    xsize = settings.value("xsize", 19).toInt();
    ysize = settings.value("ysize", 19).toInt();
    komi  = settings.value("komi", 6.5).toDouble();
    codec = mugoApp()->defaultCodec();

    // check board size radio button
    if (xsize == ysize){
      if (xsize == 19)
            ui->radio19Button->setChecked(true);
        else if (xsize == ysize && xsize == 13)
            ui->radio13Button->setChecked(true);
        else if (xsize == ysize && xsize == 9)
            ui->radio9Button->setChecked(true);
        else{
            ui->radioCustomButton->setChecked(true);
            ui->customSpinBox->setValue(xsize);
        }
    }
    else{
        ui->radioRectangularButton->setChecked(true);
        ui->xsizeSpinBox->setValue(xsize);
        ui->ysizeSpinBox->setValue(ysize);
    }

    // set komi
    ui->komiSpinBox->setValue(komi);

    // set encoding combobox
    const QList<QTextCodec*>& codecs = mugoApp()->codecs();
    QList<QTextCodec*>::const_iterator iter = codecs.begin();
    foreach(QAction* act, mugoApp()->encodingActions()){
        if (act->isSeparator())
            ui->encodingComboBox->insertSeparator(ui->encodingComboBox->count());
        else
            ui->encodingComboBox->addItem(act->text());

        if (*iter == mugoApp()->defaultCodec())
            ui->encodingComboBox->setCurrentIndex(ui->encodingComboBox->count()-1);

        ++iter;
    }
}

NewDocumentDialog::~NewDocumentDialog()
{
    delete ui;
}

/**
* slot
* ok button was clicked.
*/
void NewDocumentDialog::accept()
{
    QDialog::accept();
    if (ui->radio19Button->isChecked())
        xsize = ysize = 19;
    else if (ui->radio13Button->isChecked())
        xsize = ysize = 13;
    else if (ui->radio9Button->isChecked())
        xsize = ysize = 9;
    else if (ui->radioCustomButton->isChecked())
        xsize = ysize = ui->customSpinBox->value();
    else if (ui->radioRectangularButton->isChecked()){
        xsize = ui->xsizeSpinBox->value();
        ysize = ui->ysizeSpinBox->value();
    }

    // komi
    komi = ui->komiSpinBox->value();

    // encoding
    int encodingIndex = ui->encodingComboBox->currentIndex();
    const QList<QTextCodec*>& codecs = mugoApp()->codecs();
    codec = codecs[encodingIndex];

    QSettings settings;
    settings.setValue("komi",  komi);
    settings.setValue("xsize", xsize);
    settings.setValue("ysize", ysize);
    settings.setValue("defaultCodec", codec->name());

    mugoApp()->setDefaultCodec(codec);
}

/**
* slot
* custom radio button was toggled.
*/
void NewDocumentDialog::on_radioCustomButton_toggled(bool checked)
{
    // if custom radio button is checked, enable custom spin button.
    ui->customSpinBox->setEnabled(checked);
}

/**
* slot
* rectangular radio button was toggled.
*/
void NewDocumentDialog::on_radioRectangularButton_toggled(bool checked)
{
    // row count and column count spin box is enabled if rectangular radio button is checked.
    ui->xsizeSpinBox->setEnabled(checked);
    ui->ysizeSpinBox->setEnabled(checked);
}
