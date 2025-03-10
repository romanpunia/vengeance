# Include installation targets
install(TARGETS vitex DESTINATION lib)
install(FILES
	deps/vitex/src/vitex/layer/processors.h
	DESTINATION include/vitex/layer)
install(FILES
	deps/vitex/src/vitex/network/http.h
	deps/vitex/src/vitex/network/sqlite.h
	deps/vitex/src/vitex/network/mongo.h
	deps/vitex/src/vitex/network/pq.h
	deps/vitex/src/vitex/network/smtp.h
	DESTINATION include/vitex/network)
install(FILES
	deps/vitex/src/vitex/config.hpp
	deps/vitex/src/vitex/bindings.h
	deps/vitex/src/vitex/compute.h
	deps/vitex/src/vitex/core.h
	deps/vitex/src/vitex/layer.h
	deps/vitex/src/vitex/network.h
	deps/vitex/src/vitex/scripting.h
	deps/vitex/src/vitex/vitex.h
	DESTINATION include/vitex)
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
	src/vengeance/vengeance.h
	DESTINATION include/vengeance)