# The pofiles macro creates in some versions same name targets
# which since cmake 2.8 leads to target clashes.
# Hence force the old policy for all po directories.
# http://public.kitware.com/Bug/view.php?id=12952
cmake_policy(SET CMP0002 OLD)

find_package(Gettext REQUIRED)
get_filename_component(DIR ${CMAKE_CURRENT_LIST_FILE} PATH)

add_custom_target(pot
	COMMAND xgettext --c++ -ki18n -o ${DIR}/kguitar.pot *.cpp *.h
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/kguitar
	COMMENT "Updating translation template file"
)
add_custom_target(poupdate)

set(LANGS de;es;fr;it;ru)
foreach(lang ${LANGS})
	add_subdirectory(${lang})
	add_custom_target(poupdate_${lang}
		COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --update ${DIR}/${lang}/kguitar.po ${DIR}/kguitar.pot
		COMMENT "Updating translation file for language ${lang}"
	)
	add_dependencies(poupdate_${lang} pot)
	add_dependencies(poupdate poupdate_${lang})
endforeach(lang)
