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
			elem.second->ArrangedContainerWidth = WidthLimit - cx;
			elem.second->ArrangeSubElements(elem.second->ArrangedContainerWidth, HeightLimit, w, h);

			if (elem.second->ExpandToParent)
			{
				elem.second->ArrangedWidth = elem.second->ArrangedContainerWidth;
				elem.second->ArrangedHeight = HeightLimit;
				elem.second->ArrangedContainerHeight = HeightLimit;
			}
			else
			{
				elem.second->ArrangedWidth = w;
				elem.second->ArrangedHeight = h;
			}

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
			}
			else
			{ // 没有超出横向限制，继续向右排布
				CurRow.push_back(elem.second);
				elem.second->ArrangedRelX = cx;
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
			for (auto& elem : Row)
			{
				if (!elem->ExpandToParent)
				{
					elem->ArrangedContainerHeight = RowHeight;
				}
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

	void UIElementBase::ArrangeSubElementsAbsPos(int x, int y)
	{
		int ClientX = GetFrameWidth();
		int ClientY = GetFrameHeight();
		for (auto& elem : SubElements)
		{
			if (IsLeft(alignment))
			{
				elem.second->ArrangedAbsX = x + elem.second->ArrangedRelX + ClientX;
			}
			else if (IsRight(alignment))
			{
				elem.second->ArrangedAbsX = x + ArrangedContainerWidth - elem.second->ArrangedWidth - elem.second->ArrangedRelX - ClientX;
			}
			else
			{
				elem.second->ArrangedAbsX = x + ArrangedContainerWidth / 2 - elem.second->ArrangedWidth / 2 + elem.second->ArrangedRelX + ClientX;
			}
			if (IsTop(alignment))
			{
				elem.second->ArrangedAbsY = y + elem.second->ArrangedRelY + ClientY;
			}
			else if (IsBottom(alignment))
			{
				elem.second->ArrangedAbsY = y + ArrangedContainerHeight - elem.second->ArrangedHeight - elem.second->ArrangedRelY - ClientY;
			}
			else
			{
				elem.second->ArrangedAbsY = y + ArrangedContainerHeight / 2 - elem.second->ArrangedHeight / 2 + elem.second->ArrangedRelY + ClientY;
			}
			elem.second->ArrangeSubElementsAbsPos(elem.second->ArrangedAbsX, elem.second->ArrangedAbsY);
		}
	}

	void UIElementBase::ArrangeElements(int x, int y, int w, int h)
	{
		int cw, ch;
		ArrangeSubElements(w, h, cw, ch);
		ArrangeSubElementsAbsPos(x, y);
	}

	void UIElementBase::Render(int x, int y, int w, int h)
	{
		int ActualWidth, TotalHeight;
		ArrangeElements(x, y, w, h);

		if (!Transparent)
		{
			int FillX = ArrangedAbsX + XMargin + XBorder + 1;
			int FillY = ArrangedAbsY + YMargin + YBorder + 1;
			int FillR = ArrangedAbsX + ArrangedWidth - 1 - XMargin - XBorder - 1;
			int FillB = ArrangedAbsY + ArrangedHeight - 1 - YMargin - YBorder - 1;
			int FillW = FillR + 1 - FillX;
			int FillH = FillB + 1 - FillY;
			if (FillW > 0 && FillH > 0)
			{
				FB.FillRect(FillX, FillY, FillR, FillB, FillColor);
			}
		}

		if (XBorder > 0 || YBorder > 0)
		{
			int BorderLX = ArrangedAbsX + XMargin;
			int BorderLY = ArrangedAbsY + YMargin + YBorder;
			int BorderLR = BorderLX + XBorder;
			int BorderLB = ArrangedAbsY + ArrangedHeight - 1 - YMargin - YBorder;
			int BorderRX = ArrangedAbsX + ArrangedWidth - 1 - XMargin - XBorder;
			int BorderRY = BorderLY;
			int BorderRR = ArrangedAbsX + ArrangedWidth - 1 - XMargin;
			int BorderRB = BorderLB;
			int BorderLW = BorderLR + 1 - BorderLX;
			int BorderLH = BorderLB + 1 - BorderLY;
			int BorderRW = BorderRR + 1 - BorderRX;
			int BorderRH = BorderRB + 1 - BorderRY;
			if (XBorder > 0)
			{
				if (BorderLW > 0 && BorderLH > 0)
				{
					FB.FillRect(BorderLX, BorderLY, BorderLR, BorderLB, BorderColor);
				}
				if (BorderRW > 0 && BorderRH > 0)
				{
					FB.FillRect(BorderRX, BorderRY, BorderRR, BorderRB, BorderColor);
				}
			}
			BorderLY -= YBorder;
			BorderLB += YBorder;
			BorderRY -= YBorder;
			BorderRB += YBorder;
			BorderLW = BorderLR + 1 - BorderLX;
			BorderLH = BorderLB + 1 - BorderLY;
			BorderRW = BorderRR + 1 - BorderRX;
			BorderRH = BorderRB + 1 - BorderRY;
			if (YBorder > 0)
			{
				if (BorderLW > 0 && BorderLH > 0)
				{
					FB.FillRect(BorderLX, BorderLY, BorderLR, BorderLB, BorderColor);
				}
				if (BorderRW > 0 && BorderRH > 0)
				{
					FB.FillRect(BorderRX, BorderRY, BorderRR, BorderRB, BorderColor);
				}
			}
		}
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

	void UIElementLabel::Render(int x, int y, int w, int h)
	{
		UIElementBase::Render(x, y, w, h);

		int tw, th;
		FB.GetTextMetrics(Caption, tw, th);

		int tx, ty;
		if (IsLeft(alignment))
		{
			tx = GetFrameWidth();
		}
		else if (IsRight(alignment))
		{
			tx = ArrangedWidth - GetFrameWidth() - tw;
		}
		else
		{
			tx = x + w / 2 - tw / 2;
		}
		if (IsTop(alignment))
		{
			ty = GetFrameHeight();
		}
		else if (IsBottom(alignment))
		{
			ty = ArrangedHeight - GetFrameHeight() - th;
		}
		else
		{
			ty = y + h / 2 - th / 2;
		}
		FB.DrawText(tx, ty, Caption, true, FontColor);
	}
}
