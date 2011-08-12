#ifndef SPREADSHEETVIEW_H
#define SPREADSHEETVIEW_H

#include <QTableView>

class SpreadsheetView : public QTableView
{   
    Q_OBJECT

    public:
        SpreadsheetView(QWidget *parent = 0);
        ~SpreadsheetView();


    private:        
        QAction *export2Excel;
        QAction *changeSize;
        QAction *selectAllLabel;
        QAction *axisXInterpolate;
        QAction *axisYInterpolate;
        QAction *interpolateX;
        QAction *interpolateY;
        QAction *undoModif;
        QAction *resetModif;
        QAction *factorMulti;
        QAction *factorDiv;
        QAction *offsetPlus;
        QAction *offsetMinus;
        QAction *fillAllWith;
        QAction *copyAction;
        QAction *pasteAction;
        QAction *plotAction;
        QAction *editText;
        QAction *editBit;
        QAction *editHex;
        void createActions();
        QString getExcelCell(int row, int col);

    private slots:
        #ifdef Q_WS_WIN32
            void exportToExcel();
        #endif
        void myResize(int index, int a, int b);
        void changeLabelSize();
        void selectAll_label();
        void editAsText();
        void editAsBit();
        void editAsHex();
        void interpAxisX();
        void interpAxisY();
        void interpX();
        void interpY();
        void undoM();
        void resetM();
        void factorD();
        void factorM();
        void offsetP();
        void offsetM();
        void fillAll();
        void copy();
        void paste();
        void plot();
        void contextMenuEvent( QPoint p );
        void updateActions(QModelIndex);

};

#endif // SPREADSHEETVIEW_H
