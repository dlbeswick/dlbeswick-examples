#ifndef DSOUND_EFFECTBASE_H
#define DSOUND_EFFECTBASE_H

#include "BufferModifier.h"

// TBD: effects should be nodes rather than units added to soundmodifiers.
namespace Effect
{
	template <class Derived>
	class Base : public BufferModifier
    {
        virtual bool modify(short*& buf, int& samples, int stride) { if (!needsModify()) return false; return ((Derived&)(*this)).modify(buf, samples, stride); }
        virtual bool modify(float*& buf, int& samples, int stride) { if (!needsModify()) return false; return ((Derived&)(*this)).modify(buf, samples, stride); }

    protected:
        // Return true if the effect needs to modify the output. If false, the call to
        // 'modify' is skipped.
		virtual bool needsModify() const { return true; }
    };

	template <class T>
	struct ChannelData
	{
		ChannelData(T& owner);
		virtual ~ChannelData() {}
	};

	template <class Derived>
	class IndependentChannels : public BufferModifier
    {
    public:
        void** m_channelData;

        IndependentChannels()
        {
            m_channelData = new void*[2];
            m_channelData[0] = new typename Derived::ChannelData(*(Derived*)this);
            m_channelData[1] = new typename Derived::ChannelData(*(Derived*)this);
        }

        ~IndependentChannels()
        {
            delete m_channelData[0];
            delete m_channelData[1];
            delete[] m_channelData;
        }

        virtual bool modify(short*& buf, int& samples, int stride)
        {
        	if (!needsModify())
        		return false;

            bool modifiedLeft = ((Derived&)(*this)).modify((typename Derived::ChannelData&)m_channelData[0], buf, samples, stride);
            bool modifiedRight = ((Derived&)(*this)).modify((typename Derived::ChannelData&)m_channelData[1], buf+1, samples, stride);
            return modifiedLeft || modifiedRight;
        }

        virtual bool modify(float*& buf, int& samples, int stride)
        {
        	if (!needsModify())
        		return false;

            bool modifiedLeft = ((Derived&)(*this)).modify((typename Derived::ChannelData&)m_channelData[0], buf, samples, stride);
            bool modifiedRight = ((Derived&)(*this)).modify((typename Derived::ChannelData&)m_channelData[1], buf+1, samples, stride);
            return modifiedLeft || modifiedRight;
        }

        // implement this function in derived classes
        //void modify(typename Derived::ChannelData& channel, short* buf, int samples, int stride) {};

    protected:
        // Return true if the effect needs to modify the output. If false, the call to
        // 'modify' is skipped.
		virtual bool needsModify() const { return true; }
    };
}

#endif
