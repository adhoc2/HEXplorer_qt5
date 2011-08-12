#include "excel.h"
#include <QAbstractItemModel>

//#if defined(Q_OS_WIN32)
Excel::Excel(QString strXlsFile, QObject *parent)
{
    isInstalled = false;
    excelApplication = 0;
    excelWorkBooks = 0;
    excelWorkBook = 0;
    excelSheets = 0;
    excelSheet = 0;
    fileName = strXlsFile;
    columnName = " A";

    try {
        excelApplication = new QAxObject("Excel.Application", parent);
        if (excelApplication)
        {
            excelWorkBooks = excelApplication->querySubObject("Workbooks");
            if (excelWorkBooks)
            {
                QFile file(strXlsFile);
                if (file.exists())
                    excelWorkBook = excelWorkBooks->querySubObject("Open(const QString&)", strXlsFile);

                isInstalled = true;

            }
            else
            {
                QMessageBox::information(0, "", "初始化Excel错误,可能没有安装Office组件!");
            }
        }
        else
        {
            QMessageBox::information(0, "", "excel not installed!");
        }
    } catch (...) {}
}

Excel::~Excel()
{
    closeAll();

}

bool Excel::isOk()
{
    return isInstalled;
}

QAxObject *Excel::setFileName(const QString& strXlsFile)
{
    fileName = strXlsFile;
    if (excelWorkBooks) {
        delete excelWorkBooks;
        excelWorkBooks = excelApplication->querySubObject("Workbooks");
    }

    if (excelWorkBooks) {
        QFile file(fileName);
        if (file.exists())
            excelWorkBook = excelWorkBooks->querySubObject("Open(const QString&)", fileName);
        else {
            excelWorkBook = excelWorkBooks->querySubObject("Add()");
        }

        if (excelWorkBook)
            excelSheets = excelWorkBook->querySubObject("Sheets");
        else
            QMessageBox::information(0, "", "QAxObject workbook fail!");
    }
    return excelWorkBook;
}

void Excel::closeAll()
{
    if (excelApplication) {
        try {
            excelApplication->dynamicCall(" Quit()");
            delete excelSheet;
            delete excelSheets;
            delete excelWorkBook;
            delete excelWorkBooks;
            delete excelApplication;
            excelApplication = 0;
            excelWorkBooks = 0;
            excelWorkBook = 0;
            excelSheets = 0;
            excelSheet = 0;
        } catch (...) {}
    }
}

QAxObject *Excel::getWorkBooks()
{
    return excelWorkBooks;
}

QAxObject *Excel::getWorkBook()
{
    return excelWorkBook;

}

QAxObject *Excel::getWorkSheets()
{
    return excelSheets;
}

QAxObject *Excel::getWorkSheet()
{
    return excelSheet;

}

bool Excel::addWorkBook()
{
    if (excelWorkBooks)
    {
        excelWorkBook = excelWorkBooks->querySubObject("Add()");
        if (excelWorkBook)
        {
            excelSheet = excelWorkBook->querySubObject("Worksheets(int)", 1 );
            return true;
        }
        else
            return false;
    }
    return false;
}

bool Excel::lockUpdate()
{
    try {
        if (!excelApplication)
            return false;
        excelApplication->dynamicCall(" SetScreenUpdating(bool)", false);
    } catch (...) {}

    return true;
}

bool Excel::unlockUpdate()
{
    try {
        if (!excelApplication)
            return false;
        excelApplication->dynamicCall(" SetScreenUpdating(bool)", true);
    } catch (...) {}
    return true;
}

QAxObject *Excel::selectSheet(const QString& strSheetName)
{

    try {
        if (!excelSheets)
            return 0;
        excelSheet = excelSheets->querySubObject("Item(const QVariant&)", strSheetName);

        if (!excelSheet)
            return 0;
        return excelSheet;
    } catch (...) {
        return 0;
    }
}

bool Excel::deletesheet(const QString& strSheetName)
{
    try {
        if (!excelSheets)
            return false;
        excelSheet = excelSheets->querySubObject("Item(const QVariant&)", strSheetName);

        if (!excelSheet)
            return false;
        excelSheet->dynamicCall("delete");
        return true;
    } catch (...) {
        return 0;
    }
}

bool Excel::deleteSheetExclude(const QString& strSheetName)
{
    try {
        if (!excelSheets)
            return false;
        int nCount = excelSheets->property("Count").toInt();
        QStringList sltSheetName;
        for (int i = 1; i < nCount + 1 ; i++) {
            excelSheet = excelSheets->querySubObject("Item(int index)", i);
            if (!excelSheet) {
                continue;
            }
            sltSheetName << excelSheet->property("Name").toString();
        }

        for (int j = 0; j < sltSheetName.count(); j++) {
            if (sltSheetName[j] == strSheetName) {
                continue;
            }
            excelSheet = excelSheets->querySubObject("Item(const QVariant&)", sltSheetName[j]);
            if (!excelSheet) {
                continue;
            }
            excelSheet->dynamicCall("delete");
        }
        return true;
    } catch (...) {
        return 0;
    }
}

QAxObject *Excel::selectSheet(int nSheetIndex)
{

    try {
        if (!excelSheets)
            return 0;
        excelSheet = excelSheets->querySubObject("Item(int index)", nSheetIndex);

        if (!excelSheet)
            return 0;
        return excelSheet;
    } catch (...) {
        return 0;
    }
}

bool Excel::setHeader(QStringList headerList)
{
    try {
        int len = headerList.count();
        if (len == 0) {
            return false;
        }

        for (int i = 1; i <= len; i++) {
            setValue(1, i, headerList.at(i - 1));
        }

        return true;
    } catch (...) {
        return false;
    }
}

bool Excel::insertRow(const QString& cell)
{
    try {
        if (!excelSheet)
            return false;
        QAxObject *range = excelSheet->querySubObject("Range(const QVariant&)", QVariant(cell));
        if (!range)
            return false;

        range = range->querySubObject("EntireRow");
        if (!range) {
            return false;
        }
        range->dynamicCall("Insert");
        return true;
    } catch (...) {
        return false;
    }
}

bool Excel::deleteRow(const QString& cell)
{
    try {
        if (!excelSheet)
            return false;
        QAxObject *range = excelSheet->querySubObject("Range(const QVariant&)", QVariant(cell));
        if (!range)
            return false;

        range = range->querySubObject("EntireRow");
        if (!range) {
            return false;
        }
        range->dynamicCall("Delete");
        return true;
    } catch (...) {
        return false;
    }
}

bool Excel::setValue(const QString& cell, const QString& strValue, int nType)
{
    try {
        if (!excelSheet)
            return false;
        QAxObject *range = excelSheet->querySubObject("Range(const QVariant&)", QVariant(cell));
        if (!range)
            return false;

        range->dynamicCall("SetValue(const QVariant&)", QVariant(strValue));   //
        return true;
    } catch (...) {
        return false;
    }
}

bool Excel::setRangeValue(const QString &cell1, const QString &cell2, const QVariant &value, int nType)
{
    try {
        if (!excelSheet)
            return false;

        QAxObject *range = excelSheet->querySubObject( "Range(const QString&, const QString&)", cell1, cell2);
        if (!range)
            return false;

        range->dynamicCall("SetValue(const QVariant&)", value);
        return true;
    } catch (...) {
        return false;
    }
}

bool Excel::setValue(int row, int col, const QString& strValue)
{
    try {
        if (!excelSheet)
            return false;
        QAxObject *range = excelSheet->querySubObject("Cells(int,int)", row, col);
        if (!range)
            return false;

        range->dynamicCall("SetValue(const QVariant&)", QVariant(strValue));   //
        return true;
    } catch (...) {
        return false;
    }
}

bool Excel::setValue(int row, int col, const QString& strValue, int nType)
{
    try {
        char sCell[18];
        memset(sCell, 0, 18);
        if (col <= 26)
            sprintf(sCell, " %c", 'A' + col - 1);
        else
            sprintf(sCell, "%c%c", 'A' + col / 26 - 1, 'A' + col % 26 - 1);

        QString cell = sCell;
        columnName = columnName.toUpper() > cell.toUpper() ? columnName : cell;
        cell = cell.trimmed() + QString::number(row);
        cell = cell + ":" + cell;

        if (!excelSheet)
            return false;
        QAxObject *range = excelSheet->querySubObject("Cells(int,int)", row, col);
        if (!range)
            return false;

        if (row == 1) {
            range->dynamicCall("SetHorizontalAlignment(const QVariant&)", QVariant(-4108));
            range->dynamicCall("SetNumberFormatLocal(const QVariant&)", QVariant("@"));
        } else
            if (nType == 7 || nType == 8 || nType == 9 || nType == 11 || nType == 12 || nType == 10) //SA_dtString
                range->dynamicCall("SetNumberFormatLocal(const QVariant&)", QVariant("@"));
        if (!strValue.isNull() && !strValue.isEmpty())
            range->dynamicCall("SetValue(const QVariant&)", QVariant(strValue));

        delete range;
        return true;
    } catch (...) {
        return false;
    }
}

bool Excel::setCalce(const QString& cell, const QString& szCal)
{
    try {
        if (!excelSheet)
            return false;
        QAxObject *range = excelSheet->querySubObject("Range(const QVariant&)", QVariant(cell));
        if (!range)
            return false;

        return range->setProperty("FormulaR1C1", szCal);
    } catch (...) {
        return false;
    }
}

bool Excel::setFormula(const QString& cell, const QString& szCal)
{
    try {
        if (!excelSheet)
            return false;
        QAxObject *range = excelSheet->querySubObject("Range(const QVariant&)", QVariant(cell));
        if (!range)
            return false;

        return range->setProperty("Formula", szCal);
    } catch (...) {
        return false;
    }
}

bool Excel::setFontBold(const QString& cell1, const QString& cell2, bool blBold)
{
    try {
        if (!excelSheet)
            return false;
        QAxObject *range = excelSheet->querySubObject( "Range(const QString&, const QString&)", cell1, cell2);
        if (!range)
            return false;

        range = range->querySubObject("Font");
        if (!range) {
            return false;
        }

        return range->setProperty("Bold", blBold);
    } catch (...) {
        return false;
    }
}

bool Excel::setBackgroundColor(const QString& cell1, const QString& cell2, int color)
{
    try {
        if (!excelSheet)
            return false;
        QAxObject *range = excelSheet->querySubObject( "Range(const QString&, const QString&)", cell1, cell2);
        if (!range)
            return false;

        range = range->querySubObject("Interior");
        if (!range) {
            return false;
        }

        return range->setProperty("ColorIndex", color);
    } catch (...) {
        return false;
    }
}

bool Excel::setMergeCells(const QString& cell)
{
    try {
        if (!excelSheet)
            return false;
        QAxObject *range = excelSheet->querySubObject("Range(const QVariant&)", QVariant(cell));
        if (!range)
            return false;

        range->setProperty("HorizontalAlignment", 0xFFFFEFF4);
        range->setProperty("VerticalAlignment", 0xFFFFEFF4);
        range->setProperty("WrapText", false);
        range->setProperty("Orientation", 0);
        range->setProperty("AddIndent", false);
        range->setProperty("IndentLevel", 0);
        range->setProperty("ShrinkToFit", false);
        range->setProperty("ReadingOrder", 0xFFFFEC76);
        range->setProperty("MergeCells", true);
        return true;
    } catch (...) {
        return false;
    }
}

QVariant Excel::value(int row, int col)
{
    QVariant vValue;
    try {
        if (!excelSheet)
            return QVariant();
        QAxObject *range = excelSheet->querySubObject("Cells(int,int)", row, col);
        if (!range)
            return QVariant();

        vValue = range->property("Value2");
        delete range;

    } catch (...) {}
    return vValue;
}

QString Excel::maxColumn()
{
    return columnName.trimmed();
}

bool Excel::save()
{
    try {
        if (!excelWorkBook)
            return false;

        excelWorkBook->dynamicCall("SaveAs(const QString&)", fileName);

        return true;
    } catch (...) {
        return false;
    }
}

bool Excel::save(const QString& szFileName)
{
    try {
        if (!excelWorkBook)
            return false;

        excelWorkBook->dynamicCall("SaveAs(const QString&)", szFileName);
        //excelWorkBook->dynamicCall("Save()");

        return true;
    } catch (...) {
        return false;
    }
}

bool Excel::showExcel(bool bShow)
{
    try {
        if (!excelApplication)
            return false;
        excelApplication->setProperty("DisplayAlerts", bShow);
        excelApplication->dynamicCall("SetVisible(bool)", bShow);
        return true;
    } catch (...) {
        return false;
    }
}

void Excel::autoColumnWidth(const QString& strColumns)
{
    try {
        if (!excelSheet) {
            return;
        }
        QAxObject *columns;
        if (strColumns.isEmpty())
            columns = excelSheet->querySubObject("Columns(const QString &)", "a:z");
        else
            columns = excelSheet->querySubObject("Columns(const QString &)", strColumns);
        if (!columns) {
            QMessageBox::information(0, strColumns, "cells fail");
            return;
        }

        columns->dynamicCall("AutoFit()");
    } catch (...) {}
    return;
}

bool Excel::writeRow(int row, QVariantList &list)
{
    try {
        char sCell[18];
        memset(sCell, 0, 18);
        int col = list.count();
        if (col <= 26)
            sprintf(sCell, " %c", 'A' + col - 1);
        else
            sprintf(sCell, "%c%c", 'A' + col / 26 - 1, 'A' + col % 26 - 1);

        QString cell = sCell;
        columnName = columnName.toUpper() > cell.toUpper() ? columnName : cell;
        cell = cell.trimmed() + QString::number(row);
        QString srange = "Range(\"A" + QString::number(row) + "\",\"" + cell + "\")";
        if (!excelSheet)
            return false;
        QAxObject *range = excelSheet->querySubObject(srange.toLocal8Bit());
        if (!range)
            return false;

        range->dynamicCall("SetValue2(const QVariantList&)", QVariant(list));
        delete range;

        return true;
    } catch (...) {
        return false;
    }
}

void Excel::setStringColumn(int col, int rows)
{
    try {
        char sCell[18];
        memset(sCell, 0, 18);
        if (col <= 26)
            sprintf(sCell, " %c", 'A' + col - 1);
        else
            sprintf(sCell, "%c%c", 'A' + col / 26 - 1, 'A' + col % 26 - 1);

        QString cell = sCell;
        cell = cell.trimmed();
        QString srange = "Range(\"" + cell + "1\",\"" + cell + QString::number(rows + 1) + "\")";
        QAxObject *range = excelSheet->querySubObject(srange.toLocal8Bit());
        if (range) {
            range->dynamicCall("SetNumberFormatLocal(const QVariant&)", QVariant("@"));
            delete range;
        }
    } catch (...) {}

}

int Excel::sheetCount()
{
    try {
        if (!excelSheets)
            return 0;
        return excelSheets->property("Count").toInt();
    } catch (...) {}
    return 0;
}

QString Excel::sheetName(int index)
{
    try {
        if (!excelSheets)
            return QString();
        QAxObject *s = excelSheets->querySubObject("Item(int index)", index);
        if (s) {
            return s->property("Name").toString();

        }
    } catch (...) {}
    return QString();
}

void Excel::getRowsCols(int *rows, int *cols)
{
    try {
        if (excelSheet) {
            QAxObject *ur = excelSheet->querySubObject("UsedRange()");
            if (ur) {
                QAxObject *cs = ur->querySubObject("columns()");
                if (cs) {
                    *cols = cs->property("Count").toInt();
                }

                QAxObject *rs = ur->querySubObject("Rows()");
                if (rs) {
                    *rows = rs->property("Count").toInt();
                }
            }
        }
    } catch (...) {

    }
}

QVariantList Excel::getAll(int *rows, int *cols)
{
    QVariant result;
    char sCell[18];
    try {
        getRowsCols(rows, cols);

        memset(sCell, 0, 18);
        if (*cols <= 26)
            sprintf(sCell, "%c", 'A' + *cols - 1);
        else
            sprintf(sCell, "%c%c", 'A' + *cols / 26 - 1, 'A' + *cols % 26 - 1);

        QString cell = sCell;

        cell = cell.trimmed() + QString::number(*rows);
        QString srange = "Range(\"A1\",\"" + cell + "\")";
        if (excelSheet) {
            QAxObject *range = excelSheet->querySubObject(srange.toLocal8Bit());
            if (range) {
                result =  range->property("Value");
                delete range;
            }
        }
    } catch (...) {}
    QVariantList list = qVariantValue<QVariantList>(result);

    return list;
}

QVariantList Excel::selectAll()
{
    QVariantList result;

    //  To select all rang implemention

    return result;
}

bool Excel::clearAllContents()
{
    //  To clear all contents.
    return true;
}


//#endif

