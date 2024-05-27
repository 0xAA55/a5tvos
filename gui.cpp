#include "gui.hpp"


namespace TVOS
{
	bool IsLeft(Alignment alignment)
	{
		return int(alignment) & (int(Alignment::LeftTop) | int(Alignment::LeftCenter) | int(Alignment::LeftBottom)) ? true : false;
	}

	bool IsTop(Alignment alignment)
	{
		return int(alignment) & (int(Alignment::LeftTop) | int(Alignment::CenterTop) | int(Alignment::RightTop)) ? true : false;
	}

	bool IsRight(Alignment alignment)
	{
		return int(alignment) & (int(Alignment::RightTop) | int(Alignment::RightCenter) | int(Alignment::RightBottom)) ? true : false;
	}

	bool IsBottom(Alignment alignment)
	{
		return int(alignment) & (int(Alignment::LeftBottom) | int(Alignment::CenterBottom) | int(Alignment::RightBottom)) ? true : false;
	}

	bool IsCenter(Alignment alignment)
	{
		return int(alignment) & (int(Alignment::LeftCenter) | int(Alignment::CenterCenter) | int(Alignment::RightCenter)) ? true : false;
	}

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

	void UIElementBase::ArrangeSubElements(int WidthLimit, int HeightLimit, int& ActualWidth, int& TotalHeight)
	{
		int cx = 0;
		using ElemRowType = std::vector<std::shared_ptr<UIElementBase>>;
		std::vector<ElemRowType> RowsOfElements;

		// 存储给予控件的宽度空间
		int WidthSpace = WidthLimit;
		int HeightSpace = HeightLimit;

		WidthLimit -= GetFrameWidth() * 2;
		if (WidthLimit < 0) WidthLimit = 0;
		HeightLimit -= GetFrameHeight() * 2;
		if (HeightLimit < 0) HeightLimit = 0;

		// 先按照宽度限制将所有的子控件归类到对应的行里
		for (auto& elem : SubElements)
		{
			// 当前行
			auto& CurRow = RowsOfElements.back();

			// 取得子控件的宽度和高度
			int w, h;
			elem.second->ArrangeElements(WidthLimit - cx, HeightLimit, w, h);

			// 超出横向限制，换行
			if (cx + w >= WidthLimit)
			{
				cx = 0;
				if (CurRow.size() == 0)
				{ // 如果当前行没有任何控件就要换行，则强行插入控件。
					CurRow.push_back(elem.second);
					RowsOfElements.push_back(ElemRowType());
				}
				else
				{ // 否则换行后，插入控件到新行
					RowsOfElements.push_back(ElemRowType());
					RowsOfElements.back().push_back(elem.second);
				}
				elem.second->ArrangedRelX = cx;
				elem.second->ArrangedWidth = w;
				elem.second->ArrangedHeight = h;
			}
			else
			{ // 没有超出横向限制，继续向右排布
				CurRow.push_back(elem.second);
				elem.second->ArrangedRelX = cx;
				elem.second->ArrangedWidth = w;
				elem.second->ArrangedHeight = h;
				cx += w;
			}
		}

		// 去除末尾的空行
		while (RowsOfElements.size())
		{
			if (RowsOfElements.back().size() == 0) RowsOfElements.pop_back();
			else break;
		}

		// 开始纵向排布控件
		int cy = 0;
		ActualWidth = 0; // 统计宽度
		for (auto& Row : RowsOfElements)
		{
			int RowWidth = 0;
			int RowHeight = 0;

			// 先统计行高，顺带统计总宽度
			for (auto& elem : Row)
			{
				if (RowHeight < elem->ArrangedHeight)
				{
					RowHeight = elem->ArrangedHeight;
				}
				// 统计当前行宽
				RowWidth += elem->ArrangedWidth;
			}
			// 统计最大行宽
			if (ActualWidth < RowWidth) ActualWidth = RowWidth;

			// 再设置这行每个控件的位置
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
		ArrangeSubElements(w, h, ActualWidth, TotalHeight);

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
				int BorderX = ArrangedAbsX + XMargin;
				int BorderY = ArrangedAbsY + YMargin;
				int BorderR = ArrangedAbsX + XMargin;
				int BorderB = ArrangedAbsY + YMargin;
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
		// TODO
	}
}
