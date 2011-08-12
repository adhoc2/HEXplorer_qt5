#ifndef A2LGRAMMAR_H
#define A2LGRAMMAR_H

#include <QList>
#include <QMap>
#include <QHash>
#include "basefactory.h"

class ElementNode
{
    public :    
        QList<TokenTyp> typePar;
        QList<std::string> namePar;
        QMap<std::string, FactoryPlant<Node> *>  factoryOptNode;
        QMap<std::string, FactoryPlant<Item> *>  factoryOptItem;
};

class ElementItem
{
    public :
        QList<TokenTyp> typePar;
        QList<std::string> namePar;
};

class A2lGrammar
{
    public:
        A2lGrammar();
        QHash<QString, TokenTyp> initKeywords();

        //Nodes
        ElementNode sub_function;
        ElementNode in_measurement;
        ElementNode out_measurement;
        ElementNode annotation;
        ElementNode annotation_text;
        ElementNode axis_descr;
        ElementNode calibration_handle;
        ElementNode calibration_method;
        ElementNode characteristic;
        ElementNode compuVtab;
        ElementNode def_characteristic;
        ElementNode axis_pts;
        ElementNode record_layout;
        ElementNode compu_method;
        ElementNode measurement;
        ElementNode formula;
        ElementNode function;
        ElementNode header;
        ElementNode project;
        ElementNode module;
        ElementNode memory_segment;
        ElementNode memory_layout;
        ElementNode mod_common;
        ElementNode mod_par;

        //Items
        ElementItem addr_epk;
        ElementItem alignment_byte;
        ElementItem alignment_float32_ieee;
        ElementItem alignment_long;
        ElementItem alignment_word;
        ElementItem annotation_label;
        ElementItem annotation_origin;
        ElementItem array_size;
        ElementItem axis_pts_ref;
        ElementItem axis_pts_x;
        ElementItem axis_pts_y;
        ElementItem bit_mask;
        ElementItem byte_order;
        ElementItem calibration_handle_text;
        ElementItem coeffs;
        ElementItem coeffs_linear;
        ElementItem compu_tab_ref;
        ElementItem cpu_type;
        ElementItem customer_number;
        ElementItem default_value;
        ElementItem deposit;
        ElementItem ecu;
        ElementItem ecu_address;
        ElementItem epk;
        ElementItem extended_limits;
        ElementItem fix_axis_par;
        ElementItem fnc_values;
        ElementItem function_version;
        ElementItem format;
        ElementItem formula_inv;
        ElementItem max_refresh;
        ElementItem no_axis_pts_x;
        ElementItem no_axis_pts_y;
        ElementItem number;
        ElementItem phone_no;
        ElementItem project_no;
        ElementItem read_only;
        ElementItem ref_unit;
        ElementItem status_string_ref;
        ElementItem system_constant;
        ElementItem s_rec_layout;
        ElementItem src_addr_x;
        ElementItem src_addr_y;
        ElementItem user;
        ElementItem version;


private:
        void initSub_function();
        void initIn_measurement();
        void initOut_measurement();
        void initAnnotation();
        void initAnnotation_text();
        void initAxis_descr();
        void initCalibration_handle();
        void initCalibration_method();
        void initCharacteristic();
        void initCompuVtab();
        void initDef_characteristic();
        void initAxis_pts();
        void initRecord_layout();
        void initCompu_method();
        void initMeasurement();
        void initFormula();
        void initFunction();
        void initHeader();
        void initProject();
        void initModule();
        void initMemory_segment();
        void initMemory_layout();
        void initMod_common();
        void initMod_par();

        void initAddr_epk();
        void initAlignment_byte();
        void initAlignment_float32_ieee();
        void initAlignment_long();
        void initAlignment_word();
        void initAnnotation_label();
        void initAnnotation_origin();
        void initArray_size();
        void initAxis_pts_ref();
        void initAxis_pts_x();
        void initAxis_pts_y();
        void initBit_mask();
        void initByte_order();
        void initCalibration_handle_text();
        void initCoeffs();
        void initCoeffs_linear();
        void initCompu_tab_ref();
        void initCpu_type();
        void initCustomer_number();
        void initDefault_value();
        void initDeposit();
        void initEcu();
        void initEcu_address();
        void initEpk();
        void initExtended_limits();
        void initFix_axis_par();
        void initFnc_values();
        void initFunction_version();
        void initFormat();
        void initFormula_inv();
        void initMax_refresh();
        void initNo_axis_pts_x();
        void initNo_axis_pts_y();
        void initNumber();
        void initPhone_no();
        void initProject_no();
        void initRead_only();
        void initRef_unit();
        void initStatus_string_ref();
        void initSystem_constant();
        void initS_rec_layout();
        void initSrc_addr_x();
        void initSrc_addr_y();
        void initUser();
        void initVersion();

};

#endif // A2LGRAMMAR_H
