#ifndef DSOUND_GAIN_H
#define DSOUND_GAIN_H

class Gain
{
public:
    Gain(float db, float dbMin=-144.0) :
        _db(db),
        _dbMin(dbMin)
    {
    }

    bool operator != (const Gain& rhs) const
    {
        return _db != rhs._db;
    }

    bool operator == (const Gain& rhs) const
    {
        return _db == rhs._db;
    }

    Gain operator + (const Gain& rhs) const
    {
        Gain result = *this;
        result += rhs;
        return result;
    }

    void operator += (const Gain& rhs)
    {
        _db += rhs._db;
    }

    float db() const
    {
        return _db;
    }

    float scale() const
    {
        if (_db == 0)
            return 1.0f;
        else if (_db <= _dbMin)
            return 0.0f;
        else
            return pow(10.0f, _db / 20.0f);
    }

    void setDb(float db)
    {
    	_db = db;
    }

protected:
    float _db;
    float _dbMin;
};

#endif
