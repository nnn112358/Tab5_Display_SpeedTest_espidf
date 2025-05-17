# ソースファイルを登録
idf_component_register(
    SRCS main.cpp
    INCLUDE_DIRS "."
    )
target_link_libraries(${COMPONENT_LIB} INTERFACE "-u snd_inited -u CDAudio_Init")

