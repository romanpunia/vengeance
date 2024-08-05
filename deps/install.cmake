# Include installation targets
install(TARGETS vitex DESTINATION lib)
install(FILES
        src/vengeance/audio/effects.h
        src/vengeance/audio/filters.h
        DESTINATION include/vengeance/audio)
install(FILES
        src/vengeance/layer/components.h
        src/vengeance/layer/gui.h
        src/vengeance/layer/processors.h
        src/vengeance/layer/renderers.h
        DESTINATION include/vengeance/layer)
install(FILES
        src/vengeance/audio.h
        src/vengeance/bindings.h
        src/vengeance/trigonometry.h
        src/vengeance/physics.h
        src/vengeance/layer.h
        src/vengeance/graphics.h
        src/vengeance/scripting.h
        src/vengeance/vengeance.h
        DESTINATION include/vengeance)