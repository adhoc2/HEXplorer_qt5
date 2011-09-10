#---------------- GENERAL settings ---------------#

TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .
QT       += core gui opengl network script xml xmlpatterns
contains(QT_CONFIG, scripttools): QT += scripttools
RESOURCES += icones.qrc
RC_FILE = myappicon.rc


#-------------------------------------------------#
#------------ cl microsoft compiler --------------#
#-------------------------------------------------#

equals( QMAKE_CXX, cl) {

    # --- common config --- #

    QSCINTILLA_ROOT = ..\LIBS\QScintilla-gpl-2.5.1
    WINHOARD_ROOT = ..\LIBS\hoard-38\src
    QWT_ROOT = ..\LIBS\qwt-6.0.1
    QWT3D_ROOT = ..\LIBS\qwtplot3d

    CONFIG += embed_manifest_exe qaxcontainer
    DEFINES +=  _CRT_SECURE_NO_WARNINGS QSCINTILLA_DLL QT_DLL QWT3D_DLL QWT_DLL
    INCLUDEPATH += . "C:\Program Files\quex\quex-0.59.7" \
     .\ASAP2 \
     .\DataContainers \
     .\Quex  \
     $${QSCINTILLA_ROOT}\Qt4 \
     $${WINHOARD_ROOT}\src \
     $${QWT_ROOT}\src \
     $${QWT3D_ROOT}\include \

    # ---- debug ----- #

    CONFIG( debug, debug|release ) {
        DEFINES += MY_DEBUG
        UI_DIR = release\ui
        MOC_DIR = release\moc
        OBJECTS_DIR = release\obj
        LIBS += -l$${WINHOARD_ROOT}\winhoard \
        -l$${QSCINTILLA_ROOT}\Qt4\lib\qscintilla2d \
        -l$${QWT3D_ROOT}\lib\qwtplot3dd \
        -l$${QWT_ROOT}\lib\qwtd        

        QMAKE_CXXFLAGS_DEBUG += -openmp -DQUEX_OPTION_ASSERTS_DISABLED
    }
    else {

    # ---- release ---- #

        UI_DIR = debug\ui
        MOC_DIR = debug\moc
        OBJECTS_DIR = debug\obj
        LIBS += -l$${WINHOARD_ROOT}\winhoard \
        -l$${QSCINTILLA_ROOT}\Qt4\lib\qscintilla2 \
        -l$${QWT3D_ROOT}\lib\qwtplot3d \
        -l$${QWT_ROOT}\lib\qwt \

        QMAKE_CXXFLAGS_RELEASE -= -O2
        # only for file a2l_quex_lexer.cpp because microsoft compiler cannot compile with -Ox !!
        QMAKE_CXXFLAGS_RELEASE += -Osiy -Gs -openmp -DQUEX_OPTION_ASSERTS_DISABLED
        #QMAKE_CXXFLAGS_RELEASE += -Ox -openmp -DQUEX_OPTION_ASSERTS_DISABLED
    }
}


#---------------------------------------------#
#------------  g++ Gnu compiler --------------#
#---------------------------------------------#

equals( QMAKE_CXX, g++) {

    # --- common config --- #

    QSCINTILLA_ROOT = ../LIBS/QScintilla-gpl-2.5.1
    QWT_ROOT = ../LIBS/qwt-6.0.1
    QWT3D_ROOT = ../LIBS/qwtplot3d

    UI_DIR = ui
    MOC_DIR = moc
    OBJECTS_DIR = obj
    DEFINES +=  _CRT_SECURE_NO_WARNINGS QSCINTILLA_DLL QT_DLL QWT3D_DLL QWT_DLL
    INCLUDEPATH += . ./ASAP2 ./Quex ./DataContainers $${QWT_ROOT}/src $${QWT3D_ROOT}/include $${QSCINTILLA_ROOT}/Qt4 /Applications/quex/quex-0.59.7
    LIBS += -L$${QSCINTILLA_ROOT}/Qt4/lib \
    -L$${QWT3D_ROOT}/lib \
    -L$${QWT_ROOT}/lib

    # --- debug config --- #

    CONFIG( debug, debug|release ) {
        DEFINES += MY_DEBUG
        LIBS += -lqwtplot3d -lqwt -lgomp -lqscintilla2
        QMAKE_CXXFLAGS_DEBUG += -fopenmp
    }
    else {

    # --- debug config --- #

        LIBS += -lqwtplot3d -lqwt -lgomp -lqscintilla2
        QMAKE_CXXFLAGS_RELEASE += -O3 -fopenmp -DQUEX_OPTION_ASSERTS_DISABLED
    }
}

#-----------------------------------------#
#-------------  INPUT files --------------#
#-----------------------------------------#
HEADERS += a2l.h \
    formeditor.h \
    mdimain.h \
    chtextedit.h \
    node.h \
    item.h \
    a2ltreemodel.h \
    treemodelcompleter.h \
    dialog.h \
    chooselabel.h \
    formcompare.h \
    spreadsheetview.h \
    sptablemodel.h \
    myqlineedit.h \
    projectlistwidget.h \
    comboboxdelegate.h \
    comparemodel.h \
    choosesubset.h \
    dialogcsv.h \
    mytreeview.h \
    plot3d.h \
    plot.h \
    graph.h \
    graphmodel.h \
    canvaspicker.h \
    scrollzoomer.h \
    scrollbar.h \
    dialogupdate.h \
    graphverify.h \
    mainwindow.h \
    variantdelegate.h \
    settingstree.h \
    locationdialog.h \
    workproject.h \
    dialogexceedwb.h \
    finddialog.h \
    dialogeditastext.h \
    dialogchoosemodule.h \
    ASAP2/basefactory.h \
    ASAP2/a2lgrammar.h \
    ASAP2/lexer.h \
    ASAP2/Items/user.h \
    ASAP2/Items/system_constant.h \
    ASAP2/Items/status_string_ref.h \
    ASAP2/Items/src_addr_y.h \
    ASAP2/Items/src_addr_x.h \
    ASAP2/Items/s_rec_layout.h \
    ASAP2/Items/ref_unit.h \
    ASAP2/Items/read_only.h \
    ASAP2/Items/project_no.h \
    ASAP2/Items/phone_no.h \
    ASAP2/Items/number.h \
    ASAP2/Items/no_axis_pts_y.h \
    ASAP2/Items/no_axis_pts_x.h \
    ASAP2/Items/function_version.h \
    ASAP2/Items/formula_inv.h \
    ASAP2/Items/format.h \
    ASAP2/Items/fnc_values.h \
    ASAP2/Items/fix_axis_par.h \
    ASAP2/Items/extended_limits.h \
    ASAP2/Items/epk.h \
    ASAP2/Items/ecu_address.h \
    ASAP2/Items/ecu.h \
    ASAP2/Items/deposit.h \
    ASAP2/Items/default_value.h \
    ASAP2/Items/customer_no.h \
    ASAP2/Items/cpu_type.h \
    ASAP2/Items/compu_tab_ref.h \
    ASAP2/Items/coeffs_linear.h \
    ASAP2/Items/coeffs.h \
    ASAP2/Items/calibration_handle_text.h \
    ASAP2/Items/byte_order.h \
    ASAP2/Items/bit_mask.h \
    ASAP2/Items/axis_pts_y.h \
    ASAP2/Items/axis_pts_x.h \
    ASAP2/Items/axis_pts_ref.h \
    ASAP2/Items/array_size.h \
    ASAP2/Items/annotation_origin.h \
    ASAP2/Items/annotation_label.h \
    ASAP2/Items/alignment_word.h \
    ASAP2/Items/alignment_long.h \
    ASAP2/Items/alignment_float32_ieee.h \
    ASAP2/Items/alignment_byte.h \
    ASAP2/Items/addr_epk.h \
    ASAP2/Items/version.h \
    ASAP2/Nodes/record_layout.h \
    ASAP2/Nodes/project.h \
    ASAP2/Nodes/module.h \
    ASAP2/Nodes/mod_par.h \
    ASAP2/Nodes/mod_common.h \
    ASAP2/Nodes/memory_segment.h \
    ASAP2/Nodes/memory_layout.h \
    ASAP2/Nodes/measurement.h \
    ASAP2/Nodes/loc_measurement.h \
    ASAP2/Nodes/if_data.h \
    ASAP2/Nodes/header.h \
    ASAP2/Nodes/function.h \
    ASAP2/Nodes/formula.h \
    ASAP2/Nodes/def_characteristic.h \
    ASAP2/Nodes/compu_vtab.h \
    ASAP2/Nodes/compu_method.h \
    ASAP2/Nodes/characteristic.h \
    ASAP2/Nodes/calibration_method.h \
    ASAP2/Nodes/calibration_handle.h \
    ASAP2/Nodes/axis_pts.h \
    ASAP2/Nodes/axis_descr.h \
    ASAP2/Nodes/annotation_text.h \
    ASAP2/Nodes/annotation.h \
    ASAP2/Nodes/a2ml.h \
    ASAP2/Nodes/a2lfile.h \
    ASAP2/Nodes/ref_characteristic.h \
    dialogbitdisplay.h \
    ASAP2/Nodes/in_measurement.h \
    ASAP2/Nodes/out_measurement.h \
    ASAP2/Nodes/sub_function.h \
    measmodel.h \
    ctabwidget.h \
    formscript.h \
    dialogchooseexportformat.h \
    DataContainers/datacontainer.h \
    DataContainers/data.h \
    DataContainers/csv.h \
    DataContainers/cdfxfile.h \
    DataContainers/hexfile.h \
    DataContainers/lexerCsv.h \
    ASAP2/Items/max_refresh.h \
    dialogdatadimension.h \
    charmodel.h \
    freezetablewidget.h \
    sptablemodelHex.h \
    Quex/a2l_quex_lexer-token \
    Quex/a2l_quex_lexer-configuration \
    Quex/a2l_quex_lexer_old.qx \
    Quex/a2l_quex_lexer.qx \
    Quex/a2l_quex_lexer \
    Quex/a2l_quex_lexer-token_ids
FORMS += formeditor.ui \
    mdimain.ui \
    dialog.ui \
    formcompare.ui \
    chooselabel.ui \
    choosesubset.ui \
    dialogcsv.ui \
    graph.ui \
    graphverify.ui \
    dialogexceedwb.ui \
    dialogeditastext.ui \
    dialogchoosemodule.ui \
    dialogbitdisplay.ui \
    formscript.ui \
    dialogchooseexportformat.ui \
    dialogdatadimension.ui
SOURCES += a2l.cpp \
    formeditor.cpp \
    main.cpp \
    mdimain.cpp \
    chtextedit.cpp \
    node.cpp \
    item.cpp \
    a2ltreemodel.cpp \
    treemodelcompleter.cpp \
    dialog.cpp \
    chooselabel.cpp \
    formcompare.cpp \
    sptablemodel.cpp \
    spreadsheetview.cpp \
    myqlineedit.cpp \
    projectlistwidget.cpp \
    comboboxdelegate.cpp \
    comparemodel.cpp \
    choosesubset.cpp \
    dialogcsv.cpp \
    mytreeview.cpp \
    plot3d.cpp \
    plot.cpp \
    graph.cpp \
    graphmodel.cpp \
    canvaspicker.cpp \
    scrollzoomer.cpp \
    scrollbar.cpp \
    dialogupdate.cpp \
    graphverify.cpp \
    variantdelegate.cpp \
    settingstree.cpp \
    mainwindow.cpp \
    locationdialog.cpp \
    workproject.cpp \
    dialogexceedwb.cpp \
    finddialog.cpp \
    dialogeditastext.cpp \
    dialogchoosemodule.cpp \
    ASAP2/lexer.cpp \
    ASAP2/a2lgrammar.cpp \
    ASAP2/Items/version.cpp \
    ASAP2/Items/user.cpp \
    ASAP2/Items/system_constant.cpp \
    ASAP2/Items/status_string_ref.cpp \
    ASAP2/Items/src_addr_y.cpp \
    ASAP2/Items/src_addr_x.cpp \
    ASAP2/Items/s_rec_layout.cpp \
    ASAP2/Items/ref_unit.cpp \
    ASAP2/Items/read_only.cpp \
    ASAP2/Items/project_no.cpp \
    ASAP2/Items/phone_no.cpp \
    ASAP2/Items/number.cpp \
    ASAP2/Items/no_axis_pts_y.cpp \
    ASAP2/Items/no_axis_pts_x.cpp \
    ASAP2/Items/function_version.cpp \
    ASAP2/Items/formula_inv.cpp \
    ASAP2/Items/format.cpp \
    ASAP2/Items/fnc_values.cpp \
    ASAP2/Items/fix_axis_par.cpp \
    ASAP2/Items/extended_limits.cpp \
    ASAP2/Items/epk.cpp \
    ASAP2/Items/ecu_address.cpp \
    ASAP2/Items/ecu.cpp \
    ASAP2/Items/deposit.cpp \
    ASAP2/Items/default_value.cpp \
    ASAP2/Items/customer_no.cpp \
    ASAP2/Items/cpu_type.cpp \
    ASAP2/Items/compu_tab_ref.cpp \
    ASAP2/Items/coeffs_linear.cpp \
    ASAP2/Items/coeffs.cpp \
    ASAP2/Items/calibration_handle_text.cpp \
    ASAP2/Items/byte_order.cpp \
    ASAP2/Items/bit_mask.cpp \
    ASAP2/Items/axis_pts_y.cpp \
    ASAP2/Items/axis_pts_x.cpp \
    ASAP2/Items/axis_pts_ref.cpp \
    ASAP2/Items/array_size.cpp \
    ASAP2/Items/annotation_origin.cpp \
    ASAP2/Items/annotation_label.cpp \
    ASAP2/Items/alignment_word.cpp \
    ASAP2/Items/alignment_long.cpp \
    ASAP2/Items/alignment_float32_ieee.cpp \
    ASAP2/Items/alignment_byte.cpp \
    ASAP2/Items/addr_epk.cpp \
    ASAP2/Nodes/ref_characteristic.cpp \
    ASAP2/Nodes/record_layout.cpp \
    ASAP2/Nodes/project.cpp \
    ASAP2/Nodes/module.cpp \
    ASAP2/Nodes/mod_par.cpp \
    ASAP2/Nodes/mod_common.cpp \
    ASAP2/Nodes/memory_segment.cpp \
    ASAP2/Nodes/memory_layout.cpp \
    ASAP2/Nodes/measurement.cpp \
    ASAP2/Nodes/loc_measurement.cpp \
    ASAP2/Nodes/if_data.cpp \
    ASAP2/Nodes/header.cpp \
    ASAP2/Nodes/function.cpp \
    ASAP2/Nodes/formula.cpp \
    ASAP2/Nodes/def_characteristic.cpp \
    ASAP2/Nodes/compu_vtab.cpp \
    ASAP2/Nodes/compu_method.cpp \
    ASAP2/Nodes/characteristic.cpp \
    ASAP2/Nodes/calibration_method.cpp \
    ASAP2/Nodes/calibration_handle.cpp \
    ASAP2/Nodes/axis_pts.cpp \
    ASAP2/Nodes/axis_descr.cpp \
    ASAP2/Nodes/annotation_text.cpp \
    ASAP2/Nodes/annotation.cpp \
    ASAP2/Nodes/a2ml.cpp \
    ASAP2/Nodes/a2lfile.cpp \
    dialogbitdisplay.cpp \
    ASAP2/Nodes/in_measurement.cpp \
    ASAP2/Nodes/out_measurement.cpp \
    ASAP2/Nodes/sub_function.cpp \
    measmodel.cpp \
    ctabwidget.cpp \
    formscript.cpp \
    dialogchooseexportformat.cpp \
    DataContainers/hexfile.cpp \
    DataContainers/datacontainer.cpp \
    DataContainers/data.cpp \
    DataContainers/csv.cpp \
    DataContainers/cdfxfile.cpp \
    DataContainers/lexerCsv.cpp \
    ASAP2/Items/max_refresh.cpp \
    dialogdatadimension.cpp \
    charmodel.cpp \
    freezetablewidget.cpp \
    sptablemodelHex.cpp \
    Quex/a2l_quex_lexer.cpp





























