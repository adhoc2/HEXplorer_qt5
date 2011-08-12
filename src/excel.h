#ifndef EXCEL_H
#define EXCEL_H

#include <QtGui>
#include <QAxObject>
#include <QTableView>
#include <QStringList>

class Excel : public QObject
{

public:
    Excel(QString strXlsFile = QString(), QObject *parent = 0);
    ~Excel();

    bool isOk();
    QAxObject *getWorkBooks();
    QAxObject *getWorkBook();
    QAxObject *getWorkSheets();
    QAxObject *getWorkSheet();
    QAxObject *selectSheet(const QString& strSheetName);
    QAxObject *selectSheet(int nSheetIndex);
    QAxObject *setFileName(const QString& strFilename);
    bool addWorkBook();
    bool deletesheet(const QString& strSheetName);
    bool deleteSheetExclude(const QString& strSheetName);
    bool lockUpdate();
    bool unlockUpdate();
    bool setHeader(QStringList headerList);
    bool insertRow(const QString& cell);
    bool deleteRow(const QString& cell);
    bool setValue(const QString& cell, const QString& strValue, int nType);
    bool setRangeValue(const QString& cell1, const QString& cell2, const QVariant& value, int nType = 0);
    bool setValue(int row, int col, const QString& strValue);
    bool setValue(int row, int col, const QString& strValue, int nType);
    bool setCalce(const QString& cell, const QString& szCal);
    bool setFormula(const QString& cell, const QString& szCal);
    bool setFontBold(const QString& cell1, const QString& cell2, bool blBold);
    bool setBackgroundColor(const QString& cell1, const QString& cell2, int color);
    bool setMergeCells(const QString& cell);
    QVariant value(int row, int col);
    QVariantList getAll(int *rows, int *cols);
    QVariantList selectAll();
    bool clearAllContents();
    bool writeRow(int row,  QVariantList &list);
    void setStringColumn(int col, int rows);
    bool save();
    bool save(const QString& szFileName);
    bool showExcel(bool bShow);
    void closeAll();
    void autoColumnWidth(const QString& strColumns);
    int sheetCount();
    QString sheetName(int index);
    void getRowsCols(int *rows, int *cols);
    QString maxColumn();


protected:
    QAxObject   *excelApplication;
    QAxObject   *excelWorkBooks;
    QAxObject   *excelWorkBook;
    QAxObject   *excelSheets;
    QAxObject   *excelSheet;
    QString     fileName;
    QString     columnName;
    bool        isInstalled;


private:

};
//#if defined(Q_OS_WIN32)
//#endif

#endif
