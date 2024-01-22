add_executable(kglobalaccel-migrate)
target_sources(kglobalaccel-migrate PRIVATE
    main.cpp
    match.cpp
    move.cpp
)
target_link_libraries(kglobalaccel-migrate PRIVATE
    Qt6::Core
    KF6::ConfigCore
)
install(TARGETS kglobalaccel-migrate ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
