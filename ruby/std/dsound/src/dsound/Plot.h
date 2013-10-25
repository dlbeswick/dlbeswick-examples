#pragma once

class Plot
{
public:
	Plot(float* buffer, float* bufferEnd, int stride = 1)
	{
		// fill our own buffer with the input data
		for (float* b = buffer; b < bufferEnd; b += stride)
		{
			m_buffer.insert(m_buffer.end(), 1, *b);
		}

		m_max = *std::max_element(m_buffer.begin(), m_buffer.end());
		m_min = *std::min_element(m_buffer.begin(), m_buffer.end());
	}

	float operator() (float t) const
	{
		float v = m_buffer[(int)Mapping::linear(t, 0.0f, 1.0f, 0.0f, (float)m_buffer.size() - 1)];
		
		if (m_max - m_min != 0)
			v /= m_max - m_min;

		return v;
	}

	float max() const
	{
		return m_max;
	}

	float min() const
	{
		return m_min;
	}

protected:
	std::vector<float> m_buffer;
	float m_max;
	float m_min;
};

class DialogPlot
{
public:
	DialogPlot(const Plot& _plot, const RGBA& col = RGBA(1,1,1), int sizeX = 640, int sizeY = 480) :
		plot(_plot),
		m_sizeX(sizeX),
		m_sizeY(sizeY)
	{
		m_pen = CreatePen(PS_SOLID, 1, RGB(col.r * 255, col.g * 255, col.b * 255));
		DialogBoxParam(g_dllInstance, "BlankDialog", 0, (DLGPROC)Proc, (LPARAM)this);
	}

	~DialogPlot()
	{
		DeleteObject(m_pen);
	}

protected:
	static LRESULT Proc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		DialogPlot& self = *(DialogPlot*)GetWindowLongPtr(hDlg, DWLP_USER);

		switch (Msg)
		{
		case WM_CLOSE:
			DestroyWindow(hDlg);
			return true;

		case WM_INITDIALOG:
			{
				DialogPlot& self = *(DialogPlot*)lParam;
				SetWindowLongPtr(hDlg, DWLP_USER, lParam);
				SetWindowPos(hDlg, 0, 0, 0, self.m_sizeX, self.m_sizeY, SWP_NOMOVE);
				return true;
			}

		case WM_PAINT:
			{
				RECT client;
				GetClientRect(hDlg, &client);

				int width = client.right - client.left;
				int height = client.bottom - client.top;

				PAINTSTRUCT paint;
				BeginPaint(hDlg, &paint);

				FillRect(paint.hdc, &paint.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
				SelectObject(paint.hdc, GetStockObject(WHITE_BRUSH));
				SelectObject(paint.hdc, self.m_pen);

				float last = FLT_MAX;
				for (int i = paint.rcPaint.left; i < paint.rcPaint.right; ++i)
				{
					float val = self.plot((float)i / width) * height;
					val /= 2;

					if (last == FLT_MAX)
						last = val;

					MoveToEx(paint.hdc, i-1, height / 2 + last, 0);
					LineTo(paint.hdc, i, height / 2 + val);

					last = val;
				}

				EndPaint(hDlg, &paint);
				return true;
			}
		}

		return FALSE;
	}

	const Plot& plot;
	int m_sizeX;
	int m_sizeY;
	HPEN m_pen;
};
