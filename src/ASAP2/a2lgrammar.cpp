// HEXplorer is an Asap and HEX file editor
// Copyright (C) 2011  <Christophe Hoel>
//
// This file is part of HEXplorer.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// please contact the author at : christophe.hoel@gmail.com

#include "a2lgrammar.h"
#include "Nodes/a2lfile.h"


A2lGrammar::A2lGrammar()
{
    // initialize Nodes
    initSub_function();
    initIn_measurement();
    initOut_measurement();
    initAnnotation();
    initAnnotation_text();
    initAxis_descr();
    initAxis_pts();
    initCharacteristic();
    initCompu_method();
    initRecord_layout();
    initMeasurement();
    initCalibration_handle();
    initCalibration_method();
    initCompuVtab();
    initDef_characteristic();
    initFormula();
    initFunction();
    initHeader();
    initProject();
    initModule();
    initMemory_segment();
    initMemory_layout();
    initMod_common();
    initMod_par();

    // initialize Items
    initAddr_epk();
    initAlignment_byte();
    initAlignment_float32_ieee();
    initAlignment_long();
    initAlignment_word();
    initAnnotation_label();
    initAnnotation_origin();
    initArray_size();
    initAxis_pts_ref();
    initAxis_pts_x();
    initAxis_pts_y();
    initBit_mask();
    initByte_order();
    initCalibration_handle_text();
    initCoeffs();
    initCoeffs_linear();
    initCompu_tab_ref();
    initCpu_type();
    initCustomer_number();
    initDefault_value();
    initDeposit();
    initEcu();
    initEcu_address();
    initEpk();
    initExtended_limits();
    initFix_axis_par();
    initFnc_values();
    initFormat();
    initFormula_inv();
    initMax_refresh();
    initNo_axis_pts_x();
    initNo_axis_pts_y();
    initNumber();
    initPhone_no();
    initProject_no();
    initRead_only();
    initRef_unit();
    initStatus_string_ref();
    initSystem_constant();
    initUser();
    initVersion();
    initS_rec_layout();
    initSrc_addr_x();
    initSrc_addr_y();
    initFunction_version();
}

QHash<QString, TokenTyp> A2lGrammar::initKeywords()
{
    QHash<QString, TokenTyp> keywordsList;
    QList<std::string> list;

    list << "ASAP2_VERSION" << "A2ML_VERSION" << "AXIS_DESCR" << "AXIS_PTS_REF" << "PROJECT"
        << "PROJECT_NO" << "VERSION" << "HEADER"
        << "MODULE" << "A2ML" << "MOD_PAR" << "CHARACTERISTIC"
        << "AXIS_PTS" << "BYTE_ORDER" << "ANNOTATION" << "MEASUREMENT" << "LOC_MEASUREMENT"
        << "COMPU_TAB" << "GROUP" <<  "RECORD_LAYOUT" << "REF_CHARACTERISTIC"
        << "IF_DATA" << "SYSTEM_CONSTANT" << "EXTENDED_LIMITS" << "ANNOTATION_LABEL"
        << "ANNOTATION_ORIGIN" << "ANNOTATION_TEXT" << "FORMAT" << "ECU_ADDRESS" << "ARRAY_SIZE"
        << "BIT_MASK" << "ADDR_EPK" << "EPK" << "CUSTOMER_NO" << "USER" << "PHONE_NO" << "ECU"
        << "CPU_TYPE" << "MEMORY_SEGMENT" << "CALIBRATION_METHOD" << "CALIBRATION_HANDLE"
        << "CALIBRATION_HANDLE_TEXT" << "FUNCTION" << "MOD_COMMON" << "COMPU_METHOD" << "COMPU_VTAB"
        << "MEMORY_LAYOUT" << "S_REC_LAYOUT" << "SRC_ADDR_X" << "SRC_ADDR_Y"
        << "/begin" << "/end" << "DEPOSIT" << "NUMBER" << "READ_ONLY" << "FIX_AXIS_PAR"
        << "COEFFS" << "COEFFS_LINEAR" << "COMPU_TAB_REF" << "FORMULA" << "FORMULA_INV" << "REF_UNIT"
        << "STATUS_STRING_REF" << "DEFAULT_VALUE" << "DEF_CHARACTERISTIC" <<"OUT_MEASUREMENT"
        << "NO_AXIS_PTS_X" << "NO_AXIS_PTS_Y" << "AXIS_PTS_X" << "AXIS_PTS_Y" << "FNC_VALUES"
        << "ALIGNMENT_FLOAT32_IEEE" << "ALIGNMENT_BYTE" << "ALIGNMENT_WORD" << "ALIGNMENT_LONG"
        << "FUNCTION_VERSION" << "IN_MEASUREMENT" << "SUB_FUNCTION" << "MAX_REFRESH";

    foreach (std::string str, list)
        keywordsList.insert(QString(str.c_str()), Keyword);

    list.clear();

    //--------------------------------

    list << "ASCII" << "CURVE" << "MAP" << "CUBOID" << "CUBE_4" << "CUBE_5" << "VAL_BLK" << "VALUE";

    foreach (std::string str, list)
        keywordsList.insert(QString(str.c_str()), Type);

    list.clear();

    //--------------------------------

    list << "UBYTE" << "SBYTE" << "UWORD" << "SWORD" << "ULONG" << "SLONG" << "A_UINT64"
            << "A_INT64" << "FLOAT32_IEEE" << "FLOAT64_IEEE";

    foreach (std::string str, list)
        keywordsList.insert(QString(str.c_str()), DataType);

    list.clear();

    //--------------------------------

    list << "BYTE" << "WORD" << "LONG";

    foreach (std::string str, list)
        keywordsList.insert(QString(str.c_str()), Datasize);

    list.clear();

    //--------------------------------

    list << "PBYTE" << "PWORD" << "PLONG" << "DIRECT";

    foreach (std::string str, list)
        keywordsList.insert(QString(str.c_str()), Addrtype);

    list.clear();

    //--------------------------------

    list << "LITTLE_ENDIAN" << "BIG_ENDIAN" << "MSB_LAST" << "MSB_FIRST";

    foreach (std::string str, list)
        keywordsList.insert(QString(str.c_str()), Byteorder);

    list.clear();

    //--------------------------------

    list << "ALTERNATE_CURVES" <<"ALTERNATE_WITH_X" << "ALTERNATE_WITH_Y" << "COLUMN_DIR"
            << "ROW_DIR";

    foreach (std::string str, list)
        keywordsList.insert(QString(str.c_str()), IndexMode);

    list.clear();

    //--------------------------------

    list << "INDEX_DECR" <<"INDEX_INCR";

    foreach (std::string str, list)
        keywordsList.insert(QString(str.c_str()), Indexorder);

    list.clear();

    //--------------------------------

    list << "RAT_FUNC" << "TAB_VERB" << "IDENTICAL" << "FORM" << "LINEAR" << "TAB_INTP"
            << "TAB_NOINTP";

    foreach (std::string str, list)
        keywordsList.insert(QString(str.c_str()), ConversionType);

    list.clear();

    //--------------------------------

    list << "CURVE_AXIS" << "COM_AXIS" << "FIX_AXIS" << "RES_AXIS" << "STD_AXIS";

    foreach (std::string str, list)
        keywordsList.insert(QString(str.c_str()), Attribute);

    list.clear();

    //--------------------------------

    list << "ABSOLUTE" << "REFERENCE";

    foreach (std::string str, list)
        keywordsList.insert(QString(str.c_str()), Mode);

    list.clear();

    //--------------------------------

    list << "CALIBRATION_VARIABLES" << "CODE" << "DATA" << "EXCLUDE_FROM_FLASH" << "OFFLINE_DATA"
            << "RESERVED" << "SERAM" << "VARIABLES" << "PRG_CODE" << "PRG_DATA" << "PRG_RESERVED";

    foreach (std::string str, list)
        keywordsList.insert(QString(str.c_str()), PrgType);

    list.clear();

    //--------------------------------

    list << "EEPROM" << "EPROM" << "FLASH" << "RAM" << "ROM" << "REGISTER";

    foreach (std::string str, list)
        keywordsList.insert(QString(str.c_str()), MemoryType);

    list.clear();

    //--------------------------------

    list << "INTERN" << "EXTERN";

    foreach (std::string str, list)
        keywordsList.insert(QString(str.c_str()), MemAttribute);

    return keywordsList;
}

void A2lGrammar::initSub_function()
{

}

void A2lGrammar::initIn_measurement()
{

}

void A2lGrammar::initOut_measurement()
{

}

void A2lGrammar::initAnnotation()
{
    annotation.factoryOptNode.insert("ANNOTATION_TEXT", &ANNOTATION_TEXT::nodeFactory);

    annotation.factoryOptItem.insert("ANNOTATION_LABEL", &ANNOTATION_LABEL::itemFactory);
    annotation.factoryOptItem.insert("ANNOTATION_ORIGIN", &ANNOTATION_ORIGIN::itemFactory);
}

void A2lGrammar::initAnnotation_text()
{
    annotation_text.typePar << String;
    annotation_text.namePar << "Annotation_text";
}

void A2lGrammar::initAxis_descr()
{
    axis_descr.typePar << Attribute << Identifier << Identifier << Integer << Float << Float;
    axis_descr.namePar << "Attribute" << "InputQuantity" << "Conversion" << "MaxAxisPoints"
            << "LowerLimit" << "UpperLimit";

    axis_descr.factoryOptItem.insert("FORMAT", &FORMAT::itemFactory);
    axis_descr.factoryOptItem.insert("EXTENDED_LIMITS", &EXTENDED_LIMITS::itemFactory);
    axis_descr.factoryOptItem.insert("DEPOSIT", &DEPOSIT::itemFactory);
    axis_descr.factoryOptItem.insert("AXIS_PTS_REF", &AXIS_PTS_REF::itemFactory);
    axis_descr.factoryOptItem.insert("FIX_AXIS_PAR", &FIX_AXIS_PAR::itemFactory);
}

void A2lGrammar::initCharacteristic()
{
    characteristic.typePar << Identifier << String << Type << Hex << Identifier << Float << Identifier << Float << Float;
    characteristic.namePar << "Name" << "LongIdentifier" << "Type" << "Adress" << "Deposit" << "MaxDiff"
            << "Conversion" << "LowerLimit" << "UpperLimit";

    characteristic.factoryOptNode.insert("IF_DATA", &IF_DATA::nodeFactory);
    characteristic.factoryOptNode.insert("ANNOTATION", &ANNOTATION::nodeFactory);
    characteristic.factoryOptNode.insert("AXIS_DESCR", &AXIS_DESCR::nodeFactory);

    characteristic.factoryOptItem.insert("FORMAT", &FORMAT::itemFactory);
    characteristic.factoryOptItem.insert("EXTENDED_LIMITS", &EXTENDED_LIMITS::itemFactory);
    characteristic.factoryOptItem.insert("NUMBER", &NUMBER::itemFactory);
    characteristic.factoryOptItem.insert("READ_ONLY", &READ_ONLY::itemFactory);
    characteristic.factoryOptItem.insert("MAX_REFRESH", &MAX_REFRESH::itemFactory);

}

void A2lGrammar::initAxis_pts()
{
    axis_pts.typePar << Identifier << String << Hex << Identifier << Identifier << Float << Identifier
            << Integer << Float << Float;
    axis_pts.namePar << "Name" << "LongIdentifier" << "Adress" << "InputQuantity" << "Deposit" << "MaxDiff"
            << "Conversion" << "MaxAxisPoints" << "LowerLimit" << "UpperLimit";

    axis_pts.factoryOptNode.insert("IF_DATA", &IF_DATA::nodeFactory);
    axis_pts.factoryOptNode.insert("ANNOTATION", &ANNOTATION::nodeFactory);

    axis_pts.factoryOptItem.insert("FORMAT", &FORMAT::itemFactory);
    axis_pts.factoryOptItem.insert("EXTENDED_LIMITS", &EXTENDED_LIMITS::itemFactory);
    axis_pts.factoryOptItem.insert("DEPOSIT", &DEPOSIT::itemFactory);
}

void A2lGrammar::initRecord_layout()
{
    record_layout.typePar << Identifier;
    record_layout.namePar << "Name";

    record_layout.factoryOptItem.insert("NO_AXIS_PTS_X", &NO_AXIS_PTS_X::itemFactory);
    record_layout.factoryOptItem.insert("NO_AXIS_PTS_Y", &NO_AXIS_PTS_Y::itemFactory);
    record_layout.factoryOptItem.insert("AXIS_PTS_X", &AXIS_PTS_X::itemFactory);
    record_layout.factoryOptItem.insert("AXIS_PTS_Y", &AXIS_PTS_Y::itemFactory);
    record_layout.factoryOptItem.insert("FNC_VALUES", &FNC_VALUES::itemFactory);
    record_layout.factoryOptItem.insert("SRC_ADDR_X", &SRC_ADDR_X::itemFactory);
    record_layout.factoryOptItem.insert("SRC_ADDR_Y", &SRC_ADDR_Y::itemFactory);
}

void A2lGrammar::initCompu_method()
{
    compu_method.typePar << Identifier << String << ConversionType << StringFormat << String;
    compu_method.namePar << "Name" << "LongIdentifier" << "ConversionType" << "Format" << "Unit";

    compu_method.factoryOptNode.insert("FORMULA", &FORMULA::nodeFactory);

    compu_method.factoryOptItem.insert("COEFFS", &COEFFS::itemFactory);
    compu_method.factoryOptItem.insert("COEFFS_LINEAR", &COEFFS_LINEAR::itemFactory);
    compu_method.factoryOptItem.insert("COMPU_TAB_REF", &COMPU_TAB_REF::itemFactory);
    compu_method.factoryOptItem.insert("REF_UNIT", &REF_UNIT::itemFactory);
    compu_method.factoryOptItem.insert("STATUS_STRING_REF", &STATUS_STRING_REF::itemFactory);
}

void A2lGrammar::initMeasurement()
{

    measurement.typePar << Identifier << String << DataType << Identifier << Integer << Float << Float << Float;
    measurement.namePar << "Name" << "LongIdentifier" << "DataType" << "Conversion" << "Resolution" << "Accuracy"
            << "LowerLimit" << "UpperLimit";

    measurement.factoryOptNode.insert("IF_DATA", &IF_DATA::nodeFactory);

    measurement.factoryOptItem.insert("FORMAT", &FORMAT::itemFactory);
    measurement.factoryOptItem.insert("ECU_ADDRESS", &ECU_ADDRESS::itemFactory);
    measurement.factoryOptItem.insert("ARRAY_SIZE", &ARRAY_SIZE::itemFactory);
    measurement.factoryOptItem.insert("BIT_MASK", &BIT_MASK::itemFactory);
    measurement.factoryOptItem.insert("BYTE_ORDER", &Byte_Order::itemFactory);

}

void A2lGrammar::initCalibration_handle()
{
    calibration_handle.typePar << Hex;
    calibration_handle.namePar << "Handle";

    calibration_handle.factoryOptItem.insert("CALIBRATION_HANDLE_TEXT", &CALIBRATION_HANDLE_TEXT::itemFactory);
}

void A2lGrammar::initCalibration_method()
{
    calibration_method.typePar << String << Integer ;
    calibration_method.namePar << "Method" << "Version";
    calibration_method.factoryOptNode.insert("CALIBRATION_HANDLE", &CALIBRATION_HANDLE::nodeFactory);

}

void A2lGrammar::initCompuVtab()
{
    compuVtab.typePar << Identifier << String << ConversionType << Integer;
    compuVtab.namePar << "Name" << "LongIdentifier" << "ConversionType" << "NumberValuePairs";

    compuVtab.factoryOptItem.insert("DEFAULT_VALUE", &DEFAULT_VALUE::itemFactory);

}

void A2lGrammar::initDef_characteristic()
{

}

void A2lGrammar::initFormula()
{
    formula.typePar << String;
    formula.namePar << "foo";

    formula.factoryOptItem.insert("FORMULA_INV", &FORMULA_INV::itemFactory);
}

void A2lGrammar::initFunction()
{
    function.typePar << Identifier << String;
    function.namePar << "Name" << "LongIdentifier";

    function.factoryOptNode.insert("DEF_CHARACTERISTIC", &DEF_CHARACTERISTIC::nodeFactory);
    function.factoryOptNode.insert("REF_CHARACTERISTIC", &REF_CHARACTERISTIC::nodeFactory);
    function.factoryOptNode.insert("LOC_MEASUREMENT", &LOC_MEASUREMENT::nodeFactory);
    function.factoryOptItem.insert("FUNCTION_VERSION", &FUNCTION_VERSION::itemFactory);
    function.factoryOptNode.insert("IN_MEASUREMENT", &IN_MEASUREMENT::nodeFactory);
    function.factoryOptNode.insert("OUT_MEASUREMENT", &OUT_MEASUREMENT::nodeFactory);
    function.factoryOptNode.insert("SUB_FUNCTION", &SUB_FUNCTION::nodeFactory);
}

void A2lGrammar::initHeader()
{
    header.typePar  << String;
    header.namePar << "Comment";

    header.factoryOptItem.insert("PROJECT_NO", &PROJECT_NO::itemFactory);
    header.factoryOptItem.insert("VERSION", &VERSION::itemFactory);

}

void A2lGrammar::initProject()
{
    project.typePar << Identifier << String;
    project.namePar << "name" << "LongIdentifier";

    project.factoryOptNode.insert("HEADER", &HEADER::nodeFactory);
    project.factoryOptNode.insert("MODULE", &MODULE::nodeFactory);
}

void A2lGrammar::initModule()
{
    module.typePar << Identifier << String;
    module.namePar << "name" << "LongIdentifier";

    module.factoryOptNode.insert("MEASUREMENT", &MEASUREMENT::nodeFactory);
    module.factoryOptNode.insert("CHARACTERISTIC", &CHARACTERISTIC::nodeFactory);
    module.factoryOptNode.insert("AXIS_PTS", &AXIS_PTS::nodeFactory);
    module.factoryOptNode.insert("A2ML", &A2ML::nodeFactory);
    module.factoryOptNode.insert("MOD_PAR", &MOD_PAR::nodeFactory);
    module.factoryOptNode.insert("IF_DATA", &IF_DATA::nodeFactory);
    module.factoryOptNode.insert("FUNCTION", &FUNCTION::nodeFactory);
    module.factoryOptNode.insert("MOD_COMMON", &MOD_COMMON::nodeFactory);
    module.factoryOptNode.insert("COMPU_METHOD", &COMPU_METHOD::nodeFactory);
    module.factoryOptNode.insert("COMPU_VTAB", &COMPU_VTAB::nodeFactory);
    module.factoryOptNode.insert("RECORD_LAYOUT", &RECORD_LAYOUT::nodeFactory);

}

void A2lGrammar::initMemory_segment()
{

    memory_segment.typePar << Identifier << String << PrgType << MemoryType << MemAttribute << Hex << Hex
            << Integer << Integer << Integer << Integer << Integer;
    memory_segment.namePar << "Name" << "LongIdentifier" << "PrgType" << "Memorytype" << "Attribute" << "Address"
            << "Size" << "Offset1" << "Offset2" << "Offset3" << "Offset4" << "Offset5" ;

    memory_segment.factoryOptNode.insert("IF_DATA", &IF_DATA::nodeFactory);
}

void A2lGrammar::initMemory_layout()
{

    memory_layout.typePar << PrgType << Hex << Hex
            << Integer << Integer << Integer << Integer << Integer;
    memory_layout.namePar << "PrgType" << "Address"
            << "Size" << "Offset1" << "Offset2" << "Offset3" << "Offset4" << "Offset5" ;

    memory_layout.factoryOptNode.insert("IF_DATA", &IF_DATA::nodeFactory);
}

void A2lGrammar::initMod_common()
{
    mod_common.typePar << String;
    mod_common.namePar << "Comment";

    mod_common.factoryOptItem.insert("BYTE_ORDER", &Byte_Order::itemFactory);
    mod_common.factoryOptItem.insert("ALIGNMENT_BYTE", &ALIGNMENT_BYTE::itemFactory);
    mod_common.factoryOptItem.insert("ALIGNMENT_WORD", &ALIGNMENT_WORD::itemFactory);
    mod_common.factoryOptItem.insert("ALIGNMENT_LONG", &ALIGNMENT_LONG::itemFactory);
    mod_common.factoryOptItem.insert("S_REC_LAYOUT", &S_REC_LAYOUT::itemFactory);
    mod_common.factoryOptItem.insert("ALIGNMENT_FLOAT32_IEEE", &ALIGNMENT_FLOAT32_IEEE::itemFactory);

}

void A2lGrammar::initMod_par()
{
    mod_par.typePar << String;
    mod_par.namePar << "Comment";

    mod_par.factoryOptNode.insert("MEMORY_SEGMENT", &MEMORY_SEGMENT::nodeFactory);
    mod_par.factoryOptNode.insert("MEMORY_LAYOUT", &MEMORY_LAYOUT::nodeFactory);
    mod_par.factoryOptNode.insert("CALIBRATION_METHOD", &CALIBRATION_METHOD::nodeFactory);

    mod_par.factoryOptItem.insert("VERSION", &VERSION::itemFactory);
    mod_par.factoryOptItem.insert("ADDR_EPK", &ADDR_EPK::itemFactory);
    mod_par.factoryOptItem.insert("CUSTOMER_NO", &CUSTOMER_NO::itemFactory);
    mod_par.factoryOptItem.insert("EPK", &EPK::itemFactory);
    mod_par.factoryOptItem.insert("USER", &USER::itemFactory);
    mod_par.factoryOptItem.insert("ECU", &ECU::itemFactory);
    mod_par.factoryOptItem.insert("CPU_TYPE", &CPU_TYPE::itemFactory);
    mod_par.factoryOptItem.insert("PHONE_NO", &PHONE_NO::itemFactory);
    mod_par.factoryOptItem.insert("SYSTEM_CONSTANT", &SYSTEM_CONSTANT::itemFactory);
}

void A2lGrammar::initAddr_epk()
{
    addr_epk.typePar  << Hex;
    addr_epk.namePar << "Address";
}

void A2lGrammar::initAlignment_byte()
{
    alignment_byte.typePar  << Integer;
    alignment_byte.namePar << "AlignmentBorder";
}

void A2lGrammar::initAlignment_float32_ieee()
{
    alignment_float32_ieee.typePar  << Integer;
    alignment_float32_ieee.namePar << "AlignmentBorder";
}

void A2lGrammar::initAlignment_long()
{
    alignment_long.typePar  << Integer;
    alignment_long.namePar << "AlignmentBorder";
}

void A2lGrammar::initAlignment_word()
{
    alignment_word.typePar  << Integer;
    alignment_word.namePar << "AlignmentBorder";
}

void A2lGrammar::initAnnotation_label()
{
    annotation_label.typePar  << String;
    annotation_label.namePar << "Label";
}

void A2lGrammar::initAnnotation_origin()
{
    annotation_origin.typePar  << String;
    annotation_origin.namePar << "Origin";

}

void A2lGrammar::initArray_size()
{
    array_size.typePar  << Integer;
    array_size.namePar << "Number";
}

void A2lGrammar::initAxis_pts_ref()
{
    axis_pts_ref.typePar  << Identifier;
    axis_pts_ref.namePar << "AxisPoints";
}

void A2lGrammar::initAxis_pts_x()
{
    axis_pts_x.typePar  << Integer << DataType << Indexorder << Addrtype;
    axis_pts_x.namePar << "Position" << "Datatype" << "IndexIncr" << "Addessing";
}

void A2lGrammar::initAxis_pts_y()
{
    axis_pts_y.typePar  << Integer << DataType << Indexorder << Addrtype;
    axis_pts_y.namePar << "Position" << "Datatype" << "IndexIncr" << "Addessing";
}

void A2lGrammar::initBit_mask()
{
    bit_mask.typePar  << Hex;
    bit_mask.namePar << "Mask";
}

void A2lGrammar::initByte_order()
{
    byte_order.typePar  << Byteorder;
    byte_order.namePar << "ByteOrder";
}

void A2lGrammar::initCalibration_handle_text()
{
    calibration_handle_text.typePar  << String;
    calibration_handle_text.namePar << "Text";
}

void A2lGrammar::initCoeffs()
{
    coeffs.typePar  << Float << Float << Float << Float << Float << Float;
    coeffs.namePar << "float1" << "float2" << "float3" << "float4" << "float5" << "float6";
}

void A2lGrammar::initCoeffs_linear()
{
    coeffs_linear.typePar  << Float << Float;
    coeffs_linear.namePar << "float1" << "float2";
}

void A2lGrammar::initCompu_tab_ref()
{
    compu_tab_ref.typePar  << Identifier;
    compu_tab_ref.namePar << "ConversionTable";
}

void A2lGrammar::initCpu_type()
{
    cpu_type.typePar  << String;
    cpu_type.namePar << "CPU";
}

void A2lGrammar::initCustomer_number()
{
    customer_number.typePar  << String;
    customer_number.namePar << "Number";
}

void A2lGrammar::initDefault_value()
{
    default_value.typePar  << String;
    default_value.namePar << "display_string";
}

void A2lGrammar::initDeposit()
{
    deposit.typePar  << Mode;
    deposit.namePar << "Mode";
}

void A2lGrammar::initEcu()
{
    ecu.typePar  << String;
    ecu.namePar << "ControlUnit";
}

void A2lGrammar::initEcu_address()
{
    ecu_address.typePar  << Hex;
    ecu_address.namePar << "Address";
}

void A2lGrammar::initEpk()
{
    epk.typePar  << String;
    epk.namePar << "Identifier";
}

void A2lGrammar::initExtended_limits()
{
    extended_limits.typePar  << Float << Float;
    extended_limits.namePar << "LowerLimit" << "UpperLimit";
}

void A2lGrammar::initMax_refresh()
{
    max_refresh.typePar  << Integer << Integer;
    max_refresh.namePar << "Scaling_unit" << "Rate";
}

void A2lGrammar::initFix_axis_par()
{
    fix_axis_par.typePar  << Integer << Integer << Integer;
    fix_axis_par.namePar << "Offset" << "Shift" << "Numberapo";

}

void A2lGrammar::initFnc_values()
{
    fnc_values.typePar  << Integer << DataType << IndexMode << Addrtype;
    fnc_values.namePar << "Position" << "Datatype" << "IndexMode" << "Addesstype";
}

void A2lGrammar::initFunction_version()
{

    function_version.typePar  << String;
    function_version.namePar << "VersionIdentifier";
}

void A2lGrammar::initFormat()
{
    format.typePar  << StringFormat;
    format.namePar << "FormatString";
}

void A2lGrammar::initFormula_inv()
{
    formula_inv.typePar  << String;
    formula_inv.namePar << "goo";
}

void A2lGrammar::initNo_axis_pts_x()
{
    no_axis_pts_x.typePar  << Integer << DataType;
    no_axis_pts_x.namePar << "Position" << "Datatype";
}

void A2lGrammar::initNo_axis_pts_y()
{
    no_axis_pts_y.typePar  << Integer << DataType;
    no_axis_pts_y.namePar << "Position" << "Datatype";
}

void A2lGrammar::initNumber()
{
    number.typePar  << Integer;
    number.namePar << "Number";
}

void A2lGrammar::initPhone_no()
{
    phone_no.typePar  << String;
    phone_no.namePar << "TelNum";
}

void A2lGrammar::initProject_no()
{
    project_no.typePar  << Identifier;
    project_no.namePar << "ProjectNumber";
}

void A2lGrammar::initS_rec_layout()
{
    s_rec_layout.typePar  << Identifier;
    s_rec_layout.namePar << "Name";
}

void A2lGrammar::initRead_only()
{

}

void A2lGrammar::initRef_unit()
{
    ref_unit.typePar  << Identifier;
    ref_unit.namePar << "Unit";
}

void A2lGrammar::initStatus_string_ref()
{
    status_string_ref.typePar  << Identifier;
    status_string_ref.namePar << "ConversionTable";
}

void A2lGrammar::initSystem_constant()
{
    system_constant.typePar  << String << String;
    system_constant.namePar << "Name" << "Value";
}

void A2lGrammar::initSrc_addr_x()
{
    src_addr_x.typePar  << Integer << DataType;
    src_addr_x.namePar << "Position" << "Datatype";
}

void A2lGrammar::initSrc_addr_y()
{
    src_addr_y.typePar  << Integer << DataType;
    src_addr_y.namePar << "Position" << "Datatype";
}

void A2lGrammar::initUser()
{
    user.typePar  << String;
    user.namePar << "UserName";
}

void A2lGrammar::initVersion()
{
    version.typePar  << String;
    version.namePar << "VersionIdentifier";
}
