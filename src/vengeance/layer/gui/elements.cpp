#include "../gui.h"
#ifdef VI_RMLUI
#include <RmlUi/Core.h>
#include <Source/Core/ElementStyle.h>

namespace vitex
{
	namespace layer
	{
		namespace gui
		{
			class expand_element final : public Rml::Element
			{
			private:
				bool hidden;

			public:
				expand_element(const Rml::String& tag) : Element(tag), hidden(false)
				{
					SetPseudoClass("hidden", false);
				}
				virtual ~expand_element() = default;
				void ProcessDefaultAction(Rml::Event& event) override
				{
					Rml::Element::ProcessDefaultAction(event);
					if (event == Rml::EventId::Click && event.GetCurrentElement() == this)
					{
						hidden = !hidden;
						SetPseudoClass("hidden", hidden);
					}
				}
			};

			static Rml::ElementInstancerGeneric<expand_element>* iexpand_instancer = nullptr;
			void subsystem::create_elements() noexcept
			{
				if (!iexpand_instancer)
				{
					iexpand_instancer = core::memory::init<Rml::ElementInstancerGeneric<expand_element>>();
					Rml::Factory::RegisterElementInstancer("expand", iexpand_instancer);
				}
			}
			void subsystem::release_elements() noexcept
			{
				core::memory::deinit(iexpand_instancer);
				iexpand_instancer = nullptr;
			}
		}
	}
}
#endif