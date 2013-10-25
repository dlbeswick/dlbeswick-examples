#include "MIDIDeviceInJack.h"
#include "Standard/api.h"
#include "Standard/CriticalSectionBlock.h"
#include <jack/midiport.h>
#include <assert.h>

MIDIDeviceInJack::MIDIDeviceInJack(jack_client_t* client, const std::string& port_name) :
	MIDIDeviceJack(client, port_name),
	m_fillBufferCS(new CriticalSection)
{
}

MIDIDeviceInJack::~MIDIDeviceInJack()
{
	delete m_fillBufferCS;
}

jack_port_t* MIDIDeviceInJack::open_jack_port()
{
	assert(_client);
	return jack_port_register(_client, _port_name.c_str(), JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
}

void MIDIDeviceInJack::jack_callback(jack_nframes_t frames)
{
	void* port_buffer = jack_port_get_buffer(_port, frames);
	if (!port_buffer)
		return;

	const int num_events = jack_midi_get_event_count(port_buffer);
	if (num_events == 0)
		return;

	Critical(*m_fillBufferCS);

	for (int i = 0; i < num_events; ++i)
	{
		jack_midi_event_t event;
		if (jack_midi_event_get(&event, port_buffer, i) == 0)
		{
			assert(event.size == 3);
			if (event.size == 3)
			{
				uchar *data = (uchar*)event.buffer;

				int messageId = *data++;
				int messageData1 = *data++;
				int messageData2 = *data++;

				// some kind of init message if all zeros? -- ignore
				if (messageId != 0 || messageData1 != 0 || messageData2 != 0)
				{
					assert(_client);
					jack_time_t jack_time_us = jack_frames_to_time(_client, event.time);
					double event_time_s = jack_time_us / 1.0e6;
					m_buffer.push_back(MIDIData(messageId, messageData1, messageData2, event_time_s));
				}
			}
		}
	}
}

MIDIDeviceInJack::Buffer MIDIDeviceInJack::data()
{
	Buffer ret;

	{
		Critical(*m_fillBufferCS);
		ret = m_buffer;
		m_buffer.clear();
	}

	return ret;
}
