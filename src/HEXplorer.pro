#---------------- GENERAL settings ---------------#


TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .
QT       += core gui opengl network script xml xmlpatterns widgets concurrent printsupport axcontainer sql
contains(QT_CONFIG, scripttools): QT += scripttools
RESOURCES += icones.qrc
RC_FILE = myappicon.rc


#-------------------------------------------------#
#------------ cl microsoft compiler --------------#
#-------------------------------------------------#

equals( QMAKE_CXX, cl) {

    # --- common config --- #

    QSCINTILLA_ROOT = ../../HEXplorer_wd/LIBS/QScintilla-gpl-2.9
    WINHOARD_ROOT = ../../HEXplorer_wd/LIBS/Hoard-3.10
    QWT_ROOT = ../../HEXplorer_wd/LIBS/qwt-6.1.2
    QWT3D_ROOT = ../../HEXplorer_wd/LIBS/qwtplot3d-master_build322
    MATHGL_ROOT = ../../HEXplorer_wd/LIBS/mathgl_msvc2013

    CONFIG += embed_manifest_exe c++11
    DEFINES +=  _CRT_SECURE_NO_WARNINGS QSCINTILLA_DLL QT_DLL QWT3D_DLL QWT_DLL CL_COMPILER
    INCLUDEPATH += . "C:\quex_0.65.10\quex-0.65.10" \
     ./ASAP2 \
     ./sqlite \
     ./DataContainers \
     ./Quex  \
     $${QSCINTILLA_ROOT}/Qt4Qt5 \
     $${WINHOARD_ROOT}/src/source \
     $${QWT_ROOT}/src \
     $${QWT3D_ROOT}/include \
     $${MATHGL_ROOT}/include \

    # ---- debug ----- #
    CONFIG( debug, debug|release ) {
        DEFINES += MY_DEBUG
        UI_DIR = debug/ui
        MOC_DIR = debug/moc
        OBJECTS_DIR = debug/obj
        LIBS += -l$${QSCINTILLA_ROOT}/Qt4Qt5/lib_msvc2013/qscintilla2d \
        -l$${QWT_ROOT}/lib_msvc2013/qwtd \
        -l$${MATHGL_ROOT}/lib/mgl \
        -l$${MATHGL_ROOT}/lib/mgl-qt5 \
        -L$${QWT3D_ROOT}/lib_msvc2013/ -lqwtplot3dd -lopengl32 -lglu32 -lgdi32 \

        QMAKE_CXXFLAGS_DEBUG += -Ox -openmp
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
    }
    else {

    # ---- release ---- #
        UI_DIR = release/ui
        MOC_DIR = release/moc
        OBJECTS_DIR = release/obj
        LIBS += -l$${QSCINTILLA_ROOT}/Qt4Qt5/lib_msvc2013/qscintilla2 \
        -l$${QWT_ROOT}/lib_msvc2013/qwt \
        -l$${MATHGL_ROOT}/lib/mgl \
        -l$${MATHGL_ROOT}/lib/mgl-qt5 \
        -L$${QWT3D_ROOT}/lib_msvc2013/ -lqwtplot3d -lopengl32 -lglu32
        #-l$${WINHOARD_ROOT}/win32/libhoard

        # only for file a2l_quex_lexer.cpp because microsoft compiler cannot compile with -Ox (-Osiy -Gs instead)
        #QMAKE_CXXFLAGS_RELEASE += -Osiy -Gs -openmp -DQUEX_OPTION_ASSERTS_DISABLED
        QMAKE_CXXFLAGS_RELEASE += -Ox -openmp -DQUEX_OPTION_ASSERTS_DISABLED -DQUEX_OPTION_INFORMATIVE_BUFFER_OVERFLOW_MESSAGE
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
    }
}


#---------------------------------------------#
#------------  g++ Gnu compiler --------------#
#---------------------------------------------#

!equals( QMAKE_CXX, cl) {

    # --- common config --- #

    QSCINTILLA_ROOT = ../../HEXplorer_wd/LIBS/QScintilla-gpl-2.9
    QWT_ROOT = ../../HEXplorer_wd/LIBS/qwt-6.1.2
    QWT3D_ROOT = ../../HEXplorer_wd/LIBS/qwtplot3d-master_build322
    win32:QUEX_ROOT = "C:\quex_0.65.10\quex-0.65.10"
    MATHGL_ROOT = ../../HEXplorer_wd/LIBS/mathgl-2_3_5_1-mingw

    DEFINES +=  _CRT_SECURE_NO_WARNINGS QSCINTILLA_DLL QT_DLL QWT_DLL

    INCLUDEPATH += . \
    ./ASAP2 \
    ./sqlite  \
    ./Quex \
    ./DataContainers \
    $${QUEX_ROOT} \
    $${QWT_ROOT}/src \
    $${QSCINTILLA_ROOT}/Qt4Qt5 \
    $${MATHGL_ROOT}/include \
    $${QWT3D_ROOT}/include

    LIBS += -L$${QSCINTILLA_ROOT}/Qt4Qt5/lib \
    -L$${QWT_ROOT}/lib \
    -L$${MATHGL_ROOT}/lib \
    -L$${QWT3D_ROOT}/lib_mingw

    # --- debug config --- #
    CONFIG( debug, debug|release ) {
        UI_DIR = debug/ui
        MOC_DIR = debug/moc
        OBJECTS_DIR = debug/obj
        DEFINES += MY_DEBUG
        LIBS += -lqwtd -lgomp -lqscintilla2d -lqwtplot3dd -lopengl32 -lglu32 -lgdi32  #-lmgl -lmgl-qt5
        QMAKE_CXXFLAGS_DEBUG += -fopenmp
    }
    else {

    # --- release config --- #
        UI_DIR = release/ui
        MOC_DIR = release/moc
        OBJECTS_DIR = release/obj
        LIBS += -lqwt -lgomp -lqscintilla2 -lqwtplot3d -lopengl32 -lglu32 #-lmgl -lmgl-qt5
        QMAKE_CXXFLAGS_RELEASE += -O3 -fopenmp -DQUEX_OPTION_ASSERTS_DISABLED -DQUEX_OPTION_INFORMATIVE_BUFFER_OVERFLOW_MESSAGE
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
    dialoghttpupdate.h \
    dialogprogressdownload.h \
    ASAP2/Items/display_identifier.h \
    ASAP2/Nodes/function_list.h \
    ASAP2/Items/alignment_float64_ieee.h \
    ASAP2/Items/customer.h \
    ASAP2/Items/supplier.h \
    ASAP2/Items/data_size.h \
    ASAP2/Nodes/group.h \
    ASAP2/Items/root.h \
    ASAP2/Nodes/sub_group.h \
    ASAP2/Nodes/ref_measurement.h \
    ASAP2/Items/ecu_address_extension.h \
    ASAP2/Items/matrix_dim.h \
    ASAP2/Items/fix_axis_par_dist.h \
    DataContainers/srecfile.h \
    ASAP2/Nodes/unit.h \
    ASAP2/Items/si_exponents.h \
    ASAP2/Items/unit_conversion.h \
    ASAP2/Nodes/compu_vtab_range.h \
    ASAP2/Nodes/compu_tab.h \
    ASAP2/Items/default_value_numeric.h \
    sqlite/sqlite3.h \
    connectionwidget.h \
    ASAP2/Nodes/dbfile.h \
    workingdirectory.h \
    deletefiledialog.h \
    ASAP2/Items/static_record_layout.h \
    plot3d.h \
    DataContainers/dcmfile.h \
    DataContainers/lexerDcm.h \
    oauth2service.h \
    treedirectory.h
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
    dialogdatadimension.ui \
    dialogprogressdownload.ui \
    deletefiledialog.ui
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
    dialoghttpupdate.cpp \
    dialogprogressdownload.cpp \
    ASAP2/Items/display_identifier.cpp \
    ASAP2/Nodes/function_list.cpp \
    ASAP2/Items/alignment_float64_ieee.cpp \
    ASAP2/Items/customer.cpp \
    ASAP2/Items/supplier.cpp \
    ASAP2/Items/data_size.cpp \
    ASAP2/Nodes/group.cpp \
    ASAP2/Items/root.cpp \
    ASAP2/Nodes/sub_group.cpp \
    ASAP2/Nodes/ref_measurement.cpp \
    ASAP2/Items/ecu_address_extension.cpp \
    ASAP2/Items/matrix_dim.cpp \
    ASAP2/Items/fix_axis_par_dist.cpp \
    DataContainers/srecfile.cpp \
    ASAP2/Nodes/unit.cpp \
    ASAP2/Items/si_exponents.cpp \
    ASAP2/Items/unit_conversion.cpp \
    ASAP2/Nodes/compu_vtab_range.cpp \
    ASAP2/Nodes/compu_tab.cpp \
    Quex/a2l_quex_lexer.cpp \
    ASAP2/Items/default_value_numeric.cpp \
    sqlite/sqlite3.c \
    connectionwidget.cpp \
    ASAP2/Nodes/dbfile.cpp \
    workingdirectory.cpp \
    deletefiledialog.cpp \
    ASAP2/Items/static_record_layout.cpp \
    plot3d.cpp \
    DataContainers/dcmfile.cpp \
    DataContainers/lexerDcm.cpp \
    oauth2service.cpp \
    treedirectory.cpp

OTHER_FILES += \
    GNU_license.txt \
    update.xml






















































































