/***************************************************************************
 * Copyright (C) 2009-2015 by Pablo Daniel Pareja Obregon                  *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "printdialog.h"

#include "global.h"
#include "idocument.h"

#include <QCompleter>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QPointer>

#include <QtPrintSupport/QPrintDialog>

namespace Caneda
{
    //! \brief Constructor.
    PrintDialog::PrintDialog(IDocument *document, QWidget *parent) :
        QDialog(parent),
        m_printer(nullptr),
        m_document(document)
    {
        ui.setupUi(this);
        ui.widget->setEnabled(false);
        ui.printerChoice->setIcon(Caneda::icon("printer"));
        ui.pdfChoice->setIcon(Caneda::icon("pdf"));
        ui.fitInPageButton->setIcon(Caneda::icon("zoom-fit-best"));
        ui.browseButton->setIcon(Caneda::icon("document-open"));

        if (!m_document->printSupportsFitInPage()) {
            ui.fitInPageButton->setChecked(false);
            ui.fitInPageButton->hide();
        }

        connect(ui.printerChoice, &QRadioButton::toggled, this, &PrintDialog::onChoiceToggled);
        connect(ui.pdfChoice,     &QRadioButton::toggled, this, &PrintDialog::onChoiceToggled);
        connect(ui.browseButton,  &QToolButton::clicked,  this, &PrintDialog::onBrowseButtonClicked);

        m_printer = new QPrinter;
        m_printer->setPageOrientation(QPageLayout::Landscape);

        const QString fileName = m_document->fileName();
        if (!fileName.isEmpty()) {
            m_printer->setDocName(fileName);

            QFileInfo info(fileName);
            QString baseName = info.completeBaseName();
            QString path = info.path();
            ui.filePathEdit->setText(QDir::toNativeSeparators(path + "/"  + baseName + ".pdf"));
        }
        else {
            ui.filePathEdit->setText(QDir::toNativeSeparators(QDir::homePath()));
        }

        QCompleter *completer = new QCompleter(this);
        QFileSystemModel *model = new QFileSystemModel(completer);
        model->setRootPath(ui.filePathEdit->text());
        completer->setModel(model);
        ui.filePathEdit->setCompleter(completer);
    }

    void PrintDialog::done(int r)
    {
        if (r == QDialog::Accepted) {
            if (ui.printerChoice->isChecked()) {
                QPrintDialog *dialog = new QPrintDialog(this);
                dialog->setWindowTitle(tr("Print options"));
                dialog->setEnabledOptions(QAbstractPrintDialog::PrintShowPageSize);

                int status = dialog->exec();
                delete dialog;

                if(status == QDialog::Rejected) {
                    return;
                }
            }
            else if (ui.pdfChoice->isChecked()) {
                m_printer->setOutputFormat(QPrinter::PdfFormat);
                m_printer->setOutputFileName(ui.filePathEdit->text());
            }

            m_document->print(m_printer, ui.fitInPageButton->isChecked());
        }

        QDialog::done(r);
    }

    void PrintDialog::onChoiceToggled()
    {
        if (ui.printerChoice->isChecked()) {
            ui.widget->setEnabled(false);
        } else {
            QString path = ui.filePathEdit->text();
            ui.filePathEdit->setText(path);
            ui.widget->setEnabled(true);
        }
    }

    //! \brief Allows the user to select a file
    void PrintDialog::onBrowseButtonClicked()
    {
        QString defaultSuffix = "pdf";
        QStringList filters;
        filters << tr("PDF files (*.pdf)");

        // Create custom dialog with pdf as default suffix
        QFileDialog dialog(this, tr("Save As"));
        dialog.setFileMode(QFileDialog::AnyFile);
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.setDirectory(ui.filePathEdit->text());
        dialog.setNameFilters(filters);
        dialog.setDefaultSuffix(defaultSuffix);

        QString fileName;
        if (dialog.exec()) {
            fileName = dialog.selectedFiles().first();
        }

        if(!fileName.isEmpty()) {
            ui.filePathEdit->setText(fileName);
        }
    }

} // namespace Caneda
