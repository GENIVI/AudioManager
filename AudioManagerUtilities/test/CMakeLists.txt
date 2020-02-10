if (WITH_DLT)
    # this test works only if Utilities are built with DLT support - skip otherwise
    add_subdirectory (AmSocketHandlerTest)
else (WITH_DLT)
    message ( WARNING "CAmSocketHandlerTest requires DLT support - skipping ...")
endif (WITH_DLT)

add_subdirectory (AmSerializerTest)
