#ifndef DATA_H
#define DATA_H

#include <string>
#include <QStringList>
#include <QScriptValue>
#include <QDomElement>
#include "node.h"
class WorkProject;
class FUNCTION;
class CHARACTERISTIC;
class PROJECT;
class AXIS_PTS;
class COMPU_METHOD;
class COMPU_VTAB;
class RECORD_LAYOUT;
class AXIS_DESCR;
class HexFile;
class Csv;
class CdfxFile;

class Data : public QObject, public Node
{
    Q_OBJECT
    Q_PROPERTY (QString name  READ getName)
    Q_PROPERTY (QList<double> x  READ getXtoDouble)
    Q_PROPERTY (QList<double> y  READ getYtoDouble)
    Q_PROPERTY (QList<QList<double> > z  READ getZtoDouble)

    public:
        Data();
        Data(Node *node);
        Data(CHARACTERISTIC *node, PROJECT  *pro, HexFile *hexFile, bool modif = true);
        Data(CHARACTERISTIC *node, PROJECT *pro,  Csv *csv, bool modif = true);
        Data(CHARACTERISTIC *node, PROJECT *pro,  CdfxFile *cdfx, bool modif = true);
        Data(AXIS_PTS *node, PROJECT *pro, HexFile *hexFile, bool modif = true);
        Data(AXIS_PTS *node, PROJECT *pro,  Csv *csv, bool modif = true);
        Data(AXIS_PTS *node, PROJECT *pro,  CdfxFile *cdfx, bool modif = true);


        static bool showWeakBoundsExceeded;
        static bool allowExceedWeakBounds;

        QString getnPtsXHexa();
        QString getnPtsYHexa();
        QDomElement writeValue2Node(QDomDocument &d);
        QString getName();
        QString getComment();
        QString getSubset();
        QString getUnit();
        void hex2phys();
        QString hex2phys(QString hex, QString axis);
        QStringList hex2phys(QStringList list, QString axis);
        void phys2hex();
        QString phys2hex(QString phys, QString axis);
        QStringList phys2hex(QStringList list, QString axis);
        bool checkValues();
        int size;
        bool isModified();
        Node *getLabel();
        PROJECT *getProject();
        QString getMaxDim();
        int getMaxAxisX();
        int getMaxAxisY();

        // axisX
        bool isAxisXComparable;
        int xOrgSize;
        QString getOrgX(int i);
        QString getX(int i);
        QStringList getX();
        void setX(int i, QString str);
        void appendX(QString str);
        void removeIndexX(int i);
        void clearX();
        int xCount();
        double getAddressX();
        void resetValX(int i);
        void undoValX(int i);
        AXIS_DESCR *getAxisDescrX();
        COMPU_METHOD *getCompuMethodAxisX();
        bool checkAxisXMonotony();
        COMPU_VTAB *getCompuTabAxisX();
        QString getInputQuantityX();

        //axisY
        int yOrgSize;
        bool isAxisYComparable;
        QString getOrgY(int i);
        QString getY(int i);
        QStringList getY();
        void setY(int i, QString str);
        void appendY(QString str);
        void clearY();
        int yCount();
        double getAddressY();
        void resetValY(int i);
        void undoValY(int i);
        AXIS_DESCR *getAxisDescrY();
        COMPU_METHOD *getCompuMethodAxisY();
        bool checkAxisYMonotony();
        COMPU_VTAB *getCompuTabAxisY();
        QString getInputQuantityY();

        //axisZ
        QString getOrgZ(int i);
        QString getOrgZ(int row, int col);
        double getOrgZ(int row, int col, bool *bl);
        QString getZ(int i);
        QString getHexZ(int i);
        QString getZ(int row, int col);
        QString getHexZ(int row, int col);
        double getZ(int row, int col, bool *ok);
        QStringList getZ();
        double getMaxZ();
        void setZ(int i, QString str);
        void setZ(int row, int col, QString str);
        void setHexZ(int i, QString str);
        void setHexZ(int row, int col, QString str);
        void appendZ(QString str);
        void clearZ();
        int zCount();
        double getAddressZ();
        void resetValZ(int i);
        void undoValZ(int i);
        void resetValZ(int row, int col);
        void undoValZ(int row, int col);
        bool checkAxisZMonotony();
        COMPU_VTAB *getCompuTabAxisZ();

        bool isSizeChanged();
        void extendSize(int newDimX, int newDimY);
        void writeValue2Csv(FUNCTION *subset, QTextStream &out);
        QString getType();
        void copyAllFrom(Data *dataSrc);
        void resetAll();
        void plot(QWidget *parent = 0);
        Node *getA2lNode();
        void updateSize();
        bool isSortedByRow;
        double interp2D(double x, double y);
        void clearOldValues();
        HexFile *getHexParent();
        Csv *getCsvParent();
        CdfxFile *getCdfxParent();

    private:

        QString checkLimitsX(QString str);
        QString checkLimitsY(QString str);
        QString checkLimitsZ(QString str);
        QString checkWeakBoundsZ(QString str);
        QString getNPtsXDatatype();
        QString getNPtsYDatatype();
        QList<double> getXtoDouble();
        QList<double> getYtoDouble();
        QList<QList<double> > getZtoDouble();
        void setX(QStringList list);
        void setY(QStringList list);
        void setZ(QStringList list);
        QStringList sortByRow();
        QStringList sortByColumn();        
        QList<double> hex2dec(QStringList listHex, std::string type,  int base);
        double hex2dec(QString hex, std::string type,  int base);
        QStringList dec2hex(QList<double> listdec, std::string type,  int base);
        QString dec2hex(double dec, std::string type,  int base);
        QStringList dec2Phys(QList<double> listdec, QString axis);

        bool modifiable;
        bool sizeChanged;
        QString moduleName;
        PROJECT *project;
        Node *label;
        HexFile *hexParent;
        Csv *csvParent;
        CdfxFile *cdfxParent;
        bool displayed;
        QStringList listX;
        QStringList listY;
        QStringList listZ;
        QMap<int, QStringList> xListVal;
        QMap<int, QStringList> yListVal;
        QMap<int, QStringList> zListVal;
        RECORD_LAYOUT *record_layout;
        COMPU_METHOD *compu_methodZ;
        AXIS_DESCR *axisDescrX;
        AXIS_DESCR *axisDescrY;
        std::string datatypeX;
        std::string datatypeY;
        std::string datatypeZ;
        COMPU_VTAB *compuTabAxisX;
        COMPU_VTAB *compuTabAxisY;
        COMPU_VTAB *compuTabAxisZ;
        double addressX;
        double addressY;
        double addressZ;
        QString type;
        int precisionX;
        int precisionY;
        int precisionZ;

    public slots:
        void setX(QScriptValue value);
        void setY(QScriptValue value);
        void setZ(QScriptValue value);

        QString toString();

};


#endif // DATA_H
