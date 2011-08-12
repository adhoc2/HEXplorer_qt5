#include "dialogbitdisplay.h"

using namespace std;

DialogBitDisplay::DialogBitDisplay(ulong & value, int length, QWidget *parent) : QDialog(parent)
{
    setupUi(this);   

    myValue = &value;
    bitset<32> myBitset(static_cast<unsigned long long>(value));
    listRadio.append(radioButton_1);
    listRadio.append(radioButton_2);
    listRadio.append(radioButton_3);
    listRadio.append(radioButton_4);
    listRadio.append(radioButton_5);
    listRadio.append(radioButton_6);
    listRadio.append(radioButton_7);
    listRadio.append(radioButton_8);
    listRadio.append(radioButton_9);
    listRadio.append(radioButton_10);
    listRadio.append(radioButton_11);
    listRadio.append(radioButton_12);
    listRadio.append(radioButton_13);
    listRadio.append(radioButton_14);
    listRadio.append(radioButton_15);
    listRadio.append(radioButton_16);
    listRadio.append(radioButton_17);
    listRadio.append(radioButton_18);
    listRadio.append(radioButton_19);
    listRadio.append(radioButton_20);
    listRadio.append(radioButton_21);
    listRadio.append(radioButton_22);
    listRadio.append(radioButton_23);
    listRadio.append(radioButton_24);
    listRadio.append(radioButton_25);
    listRadio.append(radioButton_26);
    listRadio.append(radioButton_27);
    listRadio.append(radioButton_28);
    listRadio.append(radioButton_29);
    listRadio.append(radioButton_30);
    listRadio.append(radioButton_31);
    listRadio.append(radioButton_32);

    for (int i = 0; i < listRadio.length();i++)
    {

        if (i < length)
        {
            listRadio.at(i)->setChecked(myBitset.test(i));
            listRadio.at(i)->setEnabled(true);
        }
        else
        {
            listRadio.at(i)->setEnabled(false);
        }
    }
}

void DialogBitDisplay::on_buttonBox_accepted()
{
    for (int i = 0; i < listRadio.length();i++)
    {
        myBitset.set(i, listRadio.at(i)->isChecked());
    }

     *myValue = myBitset.to_ulong();
}
