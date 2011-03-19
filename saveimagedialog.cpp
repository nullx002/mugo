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
#include <QSettings>
#include <QFileDialog>
#include "mugoapp.h"
#include "saveimagedialog.h"
#include "ui_saveimagedialog.h"

/**
* Constructor
*/
SaveImageDialog::SaveImageDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SaveImageDialog)
{
    m_ui->setupUi(this);

    // initialize controls
    QSettings settings;
    m_ui->imageSizeSpinBox->setValue( settings.value("saveAsImage/imageSize", 600).toInt() );
    m_ui->coordinateCheckBox->setChecked( settings.value("saveAsImage/showCoordinate", true).toBool() );
    m_ui->monochromeCheckBox->setChecked( settings.value("saveAsImage/monochrome", false).toBool() );

    QPushButton* button = m_ui->buttonBox->button(QDialogButtonBox::Ok);
    if (button)
        button->setEnabled(false);
}

/**
* Destructor
*/
SaveImageDialog::~SaveImageDialog()
{
    delete m_ui;
}

/**
* accept
* ok button was clicked.
*/
void SaveImageDialog::accept(){
    QDialog::accept();

    imageSize      = m_ui->imageSizeSpinBox->value();
    showCoordinate = m_ui->coordinateCheckBox->isChecked();
    monochrome     = m_ui->monochromeCheckBox->isChecked();

    // save control values
    QSettings settings;
    settings.setValue("saveAsImage/imageSize", imageSize);
    settings.setValue("saveAsImage/showCoordinate", showCoordinate);
    settings.setValue("saveAsImage/monochrome", monochrome);
}

/**
* on_fileBrowseButton_clicked
* browse button was clicked.
*/
void SaveImageDialog::on_fileBrowseButton_clicked(){
    // show save dialog.
    QString selectedFilter;
    QString fname = QFileDialog::getSaveFileName(this, QString(), QString(),
        tr("PNG image(*.png);;Bitmap image(*.bmp);;JPEG image(*.jpeg *.jpg);;TIFF image(*.tiff *.tif)"),
        &selectedFilter);
    if (fname.isEmpty())
        return;

    // if extension is nothing, add default extension.
    fileInfo.setFile(fname);
    if (fileInfo.suffix().isEmpty()){
        if (selectedFilter.indexOf("*.png") >= 0)
            fileInfo.setFile(fname + ".png");
        else if (selectedFilter.indexOf("*.bmp") >= 0)
            fileInfo.setFile(fname + ".bmp");
        else if (selectedFilter.indexOf("*.jpg") >= 0)
            fileInfo.setFile(fname + ".jpg");
        else if (selectedFilter.indexOf("*.tiff") >= 0)
            fileInfo.setFile(fname + ".tiff");
    }

    m_ui->fileNameEdit->setText( fileInfo.absoluteFilePath() );
}

/**
* on_fileNameEdit_textChanged
* fileName editbox was changed.
*/
void SaveImageDialog::on_fileNameEdit_textChanged(QString str){
    // ok button is disable if file name is empty.
    QPushButton* button = m_ui->buttonBox->button(QDialogButtonBox::Ok);
    if (button == NULL)
        return;
    button->setEnabled(str.isEmpty() == false);
}
