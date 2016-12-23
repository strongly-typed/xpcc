// ----------------------------------------------------------------------------
/*
 * WARNING: This file is generated automatically from cpp_identifiers.hpp.tpl.
 * Do not edit! Please modify the corresponding XML file instead.
 */
// ----------------------------------------------------------------------------

#ifndef	{{ namespace | upper }}_IDENTIFIER_HPP
#define	{{ namespace | upper }}_IDENTIFIER_HPP

namespace {{ namespace }}
{
	namespace domain
	{
		enum Identifier
		{
		{%- for item in domains -%}
		{%- if item.id != None %}
			{{ item.name | enumElement }} = {{ item.id | enumValue }},
		{%- endif -%}
		{%- endfor %}
		};
				
		inline const char* 
		enumToString(Identifier e)
		{
			switch (e)
			{
			{%- for item in domains %}
			{%- if item.id != None %}
				case {{ item.name | enumElement }}: return "{{ item.name | enumElement }}";
			{%- endif -%}
			{%- endfor %}
				default: return "__UNKNOWN_DOMAIN__";
			}
		}
	}

	namespace container
	{
		enum class Identifier : uint8_t
		{
		{%- for item in containers %}
			{{ item.name | enumElementStrong }} = {{ item.id | enumValue }},
		{%- endfor %}
		};
	}
	
	namespace component
	{
		enum Identifier
		{
		{%- for item in components %}
			{{ item.name | enumElement }} = {{ item.id | enumValue }},
		{%- endfor %}
		};
				
		inline const char* 
		enumToString(Identifier e)
		{
			switch (e)
			{
			{%- for item in components %}
				case {{ item.name | enumElement }}: return "{{ item.name | enumElement }}";
			{%- endfor %}
				default: return "__UNKNOWN_COMPONENT__";
			}
		}
	}
	
	/** Looks up in which container a component is instantiated */
	inline uint8_t /* container::Identifier */
	containerLut(const /* component::Identifier */ uint8_t component)
	{
		switch(component)
		{
			case 0: // Events are broadcasts to component 0, so use container 0, too.
				return 0;
		{%- for container in containers %}
			{%- for component in container.components %}
			case component::{{ component.name | enumElement }}:
			{%- endfor %}
				return static_cast<uint8_t>(container::Identifier::{{ container.name | enumElementStrong }});
		{%- endfor %}
			default:
				// Error.
				return 0xff;
		}
	}

	namespace action
	{
		enum Identifier
		{
		{%- for item in actions %}
			{{ item.name | enumElement }} = {{ item.id | enumValue }},
		{%- endfor %}
		};
				
		inline const char* 
		enumToString(Identifier e)
		{
			switch (e)
			{
			{%- for item in actions %}
				case {{ item.name | enumElement }}: return "{{ item.name | enumElement }}";
			{%- endfor %}
				default: return "__UNKNOWN_ACTION__";
			}
		}
	}
		
	namespace event
	{
		enum Identifier
		{
		{%- for item in events %}
			{{ item.name | enumElement }} = {{ item.id | enumValue }},
		{%- endfor %}
		};
		
		inline const char* 
		enumToString(Identifier e)
		{
			switch (e)
			{
			{%- for item in events %}
				case {{ item.name | enumElement }}: return "{{ item.name | enumElement }}";
			{%- endfor %}
				default: return "__UNKNOWN_EVENT__";
			}
		}
	}
}	// namespace {{ namespace }}

#endif	// {{ namespace | upper }}_IDENTIFIER_HPP
