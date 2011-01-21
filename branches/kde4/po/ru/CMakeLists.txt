file(GLOB _po_files *.po)
get_filename_component(DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
gettext_process_po_files(${DIR} ALL INSTALL_DESTINATION ${LOCALE_INSTALL_DIR} ${_po_files})
