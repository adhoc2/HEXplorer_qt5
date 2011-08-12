#include "dialogbitdisplay.h"

using namespace std;

DialogBitDisplay::DialogBitDisplay(ulong & value, QWidget *parent) : QDialog(parent)
{
    setupUi(this);   

    myValue = &value;
    bitset<32> myBitset(static_cast<unsigned long long>(value));

    radioButton->setChecked(myBitset.test(0));
    radioButton_2->setChecked(myBitset.test(1));
    radioButton_3->setChecked(myBitset.test(2));
    radioButton_4->setChecked(myBitset.test(3));
    radioButton_5->setChecked(myBitset.test(4));
    radioButton_6->setChecked(myBitset.test(5));
    radioButton_7->setChecked(myBitset.test(6));
    radioButton_8->setChecked(myBitset.test(7));
    radioButton_9->setChecked(myBitset.test(8));
    radioButton_10->setChecked(myBitset.test(9));
    radioButton_11->setChecked(myBitset.test(10));
    radioButton_12->setChecked(myBitset.test(11));
    radioButton_13->setChecked(myBitset.test(12));
    radioButton_14->setChecked(myBitset.test(13));
    radioButton_15->setChecked(myBitset.test(14));
    radioButton_16->setChecked(myBitset.test(15));

    radioButton_17->setChecked(myBitset.test(16));
    radioButton_18->setChecked(myBitset.test(17));
    radioButton_19->setChecked(myBitset.test(18));
    radioButton_20->setChecked(myBitset.test(19));
    radioButton_21->setChecked(myBitset.test(20));
    radioButton_22->setChecked(myBitset.test(21));
    radioButton_23->setChecked(myBitset.test(21));
    radioButton_24->setChecked(myBitset.test(23));
    radioButton_25->setChecked(myBitset.test(24));
    radioButton_26->setChecked(myBitset.test(25));
    radioButton_27->setChecked(myBitset.test(26));
    radioButton_28->setChecked(myBitset.test(27));
    radioButton_29->setChecked(myBitset.test(28));
    radioButton_30->setChecked(myBitset.test(29));
    radioButton_31->setChecked(myBitset.test(30));
    radioButton_32->setChecked(myBitset.test(31));

}

void DialogBitDisplay::on_buttonBox_accepted()
{
     myBitset.set(0, radioButton->isChecked());
     myBitset.set(1, radioButton_2->isChecked());
     myBitset.set(2, radioButton_3->isChecked());
     myBitset.set(3, radioButton_4->isChecked());
     myBitset.set(4, radioButton_5->isChecked());
     myBitset.set(5, radioButton_6->isChecked());
     myBitset.set(6, radioButton_7->isChecked());
     myBitset.set(7, radioButton_8->isChecked());
     myBitset.set(8, radioButton_9->isChecked());
     myBitset.set(9, radioButton_10->isChecked());
     myBitset.set(10, radioButton_11->isChecked());
     myBitset.set(11, radioButton_12->isChecked());
     myBitset.set(12, radioButton_13->isChecked());
     myBitset.set(13, radioButton_14->isChecked());
     myBitset.set(14, radioButton_15->isChecked());
     myBitset.set(15, radioButton_16->isChecked());

     myBitset.set(16, radioButton_17->isChecked());
     myBitset.set(17, radioButton_18->isChecked());
     myBitset.set(18, radioButton_19->isChecked());
     myBitset.set(19, radioButton_20->isChecked());
     myBitset.set(20, radioButton_21->isChecked());
     myBitset.set(21, radioButton_22->isChecked());
     myBitset.set(22, radioButton_23->isChecked());
     myBitset.set(23, radioButton_24->isChecked());
     myBitset.set(24, radioButton_25->isChecked());
     myBitset.set(25, radioButton_26->isChecked());
     myBitset.set(26, radioButton_27->isChecked());
     myBitset.set(27, radioButton_28->isChecked());
     myBitset.set(28, radioButton_29->isChecked());
     myBitset.set(29, radioButton_30->isChecked());
     myBitset.set(30, radioButton_31->isChecked());
     myBitset.set(31, radioButton_32->isChecked());

     *myValue = myBitset.to_ulong();

}
