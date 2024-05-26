#include "gui.hpp"


namespace TVOS
{
	UIElementBase::UIElementBase(Graphics& FB, const std::string& Name) :
		FB(FB),
		Name(Name)
	{
	}
	const std::string& UIElementBase::GetName() const
	{
		return Name;
	}

	std::shared_ptr<UIElementBase> UIElementBase::FindElement(const std::string& Name)
	{
		if (!SubElements.size()) return nullptr;
		if (SubElements.count(Name)) return SubElements.at(Name);
		for (auto& elem : SubElements)
		{
			auto find = elem.second->FindElement(Name);
			if (find) return find;
		}
		return nullptr;
	}

	int UIElementBase::GetFrameWidth() const
	{
		return XPadding + XBorder + XMargin;
	}

	int UIElementBase::GetFrameHeight() const
	{
		return YPadding + YBorder + YMargin;
	}

	void UIElementBase::ArrangeElements(int WidthLimit, int HeightLimit, int& ActualWidth, int& TotalHeight)
	{
		int cx = 0;
		using ElemRowType = std::vector<std::shared_ptr<UIElementBase>>;
		std::vector<ElemRowType> RowsOfElements;

		// �洢����ؼ��Ŀ�ȿռ�
		int WidthSpace = WidthLimit;
		int HeightSpace = HeightLimit;

		WidthLimit -= GetFrameWidth() * 2;
		if (WidthLimit < 0) WidthLimit = 0;
		HeightLimit -= GetFrameHeight() * 2;
		if (HeightLimit < 0) HeightLimit = 0;

		// �Ȱ��տ�����ƽ����е��ӿؼ����ൽ��Ӧ������
		for (auto& elem : SubElements)
		{
			// ��ǰ��
			auto& CurRow = RowsOfElements.back();

			// ȡ���ӿؼ��Ŀ�Ⱥ͸߶�
			int w, h;
			elem.second->ArrangeElements(WidthLimit - cx, HeightLimit, w, h);

			// �����������ƣ�����
			if (cx + w >= WidthLimit)
			{
				cx = 0;
				if (CurRow.size() == 0)
				{ // �����ǰ��û���κοؼ���Ҫ���У���ǿ�в���ؼ���
					CurRow.push_back(elem.second);
					RowsOfElements.push_back(ElemRowType());
				}
				else
				{ // �����к󣬲���ؼ�������
					RowsOfElements.push_back(ElemRowType());
					RowsOfElements.back().push_back(elem.second);
				}
				elem.second->ArrangedRelX = cx;
				elem.second->ArrangedWidth = w;
				elem.second->ArrangedHeight = h;
			}
			else
			{ // û�г����������ƣ����������Ų�
				CurRow.push_back(elem.second);
				elem.second->ArrangedRelX = cx;
				elem.second->ArrangedWidth = w;
				elem.second->ArrangedHeight = h;
				cx += w;
			}
		}

		// ȥ��ĩβ�Ŀ���
		while (RowsOfElements.size())
		{
			if (RowsOfElements.back().size() == 0) RowsOfElements.pop_back();
			else break;
		}

		// ��ʼ�����Ų��ؼ�
		int cy = 0;
		ActualWidth = 0; // ͳ�ƿ��
		for (auto& Row : RowsOfElements)
		{
			int RowWidth = 0;
			int RowHeight = 0;

			// ��ͳ���иߣ�˳��ͳ���ܿ��
			for (auto& elem : Row)
			{
				if (RowHeight < elem->ArrangedHeight)
				{
					RowHeight = elem->ArrangedHeight;
				}
				// ͳ�Ƶ�ǰ�п�
				RowWidth += elem->ArrangedWidth;
			}
			// ͳ������п�
			if (ActualWidth < RowWidth) ActualWidth = RowWidth;

			// ����������ÿ���ؼ���λ��
			for (auto& elem : Row)
			{
				elem->ArrangedRelY = cy;
			}

			cy += RowHeight;
		}
		TotalHeight = cy + GetFrameHeight();
	}

	void UIElementBase::Render(int x, int y, int w, int h)
	{
		int ActualWidth, TotalHeight;
		ArrangeElements(w, h, ActualWidth, TotalHeight);

		do
		{
			if (!Transparent)
			{
				int FillX = ArrangedAbsX + XMargin + XBorder + 1;
				int FillY = ArrangedAbsY + YMargin + YBorder + 1;
				int FillR = ArrangedAbsX + ArrangedWidth - 1 + XMargin + XBorder - 1;
				int FillB = ArrangedAbsY + ArrangedHeight - 1 + YMargin + YBorder - 1;
				int FillW = FillR + 1 - FillX;
				int FillH = FillB + 1 - FillY;
				if (FillW <= 0 || FillH <= 0) break;
				FB.FillRect(FillX, FillY, FillR, FillB, FillColor);
			}
		} while (false);

		do
		{
			if (XBorder)
			{

			}
		} while (false);
	}

	UIElementLabel::UIElementLabel(Graphics& FB, const std::string& Name) :
		UIElementBase(FB, Name)
	{
	}

	void UIElementLabel::SetCaption(const std::string& Caption)
	{
		this->Caption = Caption;
		FB.GetTextMetrics(Caption, CaptionWidth, CaptionHeight);
	}

	const std::string& UIElementLabel::GetCaption() const
	{
		return Caption;
	}

	void UIElementLabel::GetCaptionSize(int& w, int& h) const
	{
		w = CaptionWidth;
		h = CaptionHeight;
	}

	void UIElementLabel::Render(int x, int y, int w, int h) const
	{
		
	}
}
