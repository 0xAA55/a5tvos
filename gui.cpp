#include "gui.hpp"


namespace TVOS
{
	bool IsLeft(AlignmentType alignment)
	{
		switch (alignment)
		{
		case AlignmentType::LeftTop:
		case AlignmentType::LeftCenter:
		case AlignmentType::LeftBottom:
			return true;
		default:
			return false;
		}
	}

	bool IsTop(AlignmentType alignment)
	{
		switch (alignment)
		{
		case AlignmentType::LeftTop:
		case AlignmentType::CenterTop:
		case AlignmentType::RightTop:
			return true;
		default:
			return false;
		}
	}

	bool IsRight(AlignmentType alignment)
	{
		switch (alignment)
		{
		case AlignmentType::RightTop:
		case AlignmentType::RightCenter:
		case AlignmentType::RightBottom:
			return true;
		default:
			return false;
		}
	}

	bool IsBottom(AlignmentType alignment)
	{
		switch (alignment)
		{
		case AlignmentType::LeftBottom:
		case AlignmentType::CenterBottom:
		case AlignmentType::RightBottom:
			return true;
		default:
			return false;
		}
	}

	bool IsCenter(AlignmentType alignment)
	{
		switch (alignment)
		{
		case AlignmentType::LeftCenter:
		case AlignmentType::CenterCenter:
		case AlignmentType::RightCenter:
			return true;
		default:
			return false;
		}
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
		if (SubElementsMap.count(Name))
		{
			return SubElementsMap.at(Name);
		}
		for (auto& elem : SubElements)
		{
			auto find = elem->FindElement(Name);
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

	void UIElementBase::GetClientContentsSize(int WidthLimit, int HeightLimit, int& ActualWidth, int& TotalHeight)
	{
		int cx = 0;
		using ElemRowType = std::vector<std::shared_ptr<UIElementBase>>;
		std::vector<ElemRowType> RowsOfElements;

		auto WidthSpace = WidthLimit;
		auto HeightSpace = HeightLimit;

		WidthLimit -= GetFrameWidth() * 2;
		if (WidthLimit < 0) WidthLimit = 0;
		HeightLimit -= GetFrameHeight() * 2;
		if (HeightLimit < 0) HeightLimit = 0;

		// 先按照宽度限制将所有的子控件归类到对应的行里
		RowsOfElements.push_back(ElemRowType());
		for (auto& elem : SubElements)
		{
			// 当前行
			auto& CurRow = RowsOfElements.back();

			bool LineBreak = elem->LineBreak;

			// 取得子控件的宽度和高度
			int w, h;
			elem->ArrangedContainerWidth = WidthLimit - cx;
			elem->GetClientContentsSize(elem->ArrangedContainerWidth, HeightLimit, w, h);
			elem->ArrangedContentsWidth = w;
			elem->ArrangedContentsHeight = h;

			if (elem->ExpandToParentX)
			{
				elem->ArrangedWidth = elem->ArrangedContainerWidth - XPadding * 2;
				LineBreak = true;
			}
			else
			{
				elem->ArrangedWidth = w + elem->XPadding * 2;
			}
			if (elem->ExpandToParentY)
			{
				elem->ArrangedHeight = HeightLimit;
				elem->ArrangedContainerHeight = HeightLimit;
			}
			else
			{
				elem->ArrangedHeight = h + elem->YPadding * 2;
			}

			// 超出横向限制，换行
			w += elem->XPadding * 2;
			if (LineBreak || cx + w >= WidthLimit)
			{
				cx = 0;
				if (CurRow.size() == 0)
				{ // 如果当前行没有任何控件就要换行，则强行插入控件。
					CurRow.push_back(elem);
					RowsOfElements.push_back(ElemRowType());
					elem->ArrangedRelX = cx + XPadding;
				}
				else
				{ // 否则换行后，插入控件到新行
					RowsOfElements.push_back(ElemRowType());
					RowsOfElements.back().push_back(elem);
					elem->ArrangedRelX = cx + XPadding;
					cx += w;
				}
			}
			else
			{ // 没有超出横向限制，继续向右排布
				CurRow.push_back(elem);
				elem->ArrangedRelX = cx + XPadding;
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
		TotalHeight = 0;
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
				if (!ExpandToParentY)
				{
					elem->ArrangedContainerHeight = RowHeight;
				}
			}
			// 统计最大行宽
			if (ActualWidth < RowWidth) ActualWidth = RowWidth;

			// 再设置这行每个控件的位置
			for (auto& elem : Row)
			{
				elem->ArrangedRelY = TotalHeight + YPadding;
			}

			TotalHeight += RowHeight;
		}

		ArrangedRelX = 0;
		ArrangedRelY = 0;
		ArrangedContentsWidth = ActualWidth;
		ArrangedContentsHeight = TotalHeight;
		ArrangedContainerWidth = WidthSpace;
		ArrangedContainerHeight = HeightSpace;
		if (ExpandToParentX)
			ArrangedWidth = WidthSpace;
		else
			ArrangedWidth = ArrangedContentsWidth;
		if (ExpandToParentY)
			ArrangedHeight = HeightSpace;
		else
			ArrangedHeight = ArrangedContentsHeight;
	}

	void UIElementBase::ArrangeSubElementsAbsPos(int x, int y)
	{
		int ClientX = GetFrameWidth();
		int ClientY = GetFrameHeight();
		for (auto& elem : SubElements)
		{
			if (IsLeft(Alignment))
			{
				elem->ArrangedAbsX = x + ClientX + elem->ArrangedRelX;
			}
			else if (IsRight(Alignment))
			{
				elem->ArrangedAbsX = x + ArrangedContainerWidth - elem->ArrangedWidth - elem->ArrangedRelX - ClientX;
			}
			else
			{
				elem->ArrangedAbsX = x + ArrangedContainerWidth / 2 - ArrangedContentsWidth / 2 + elem->ArrangedRelX;
			}
			if (IsTop(Alignment))
			{
				elem->ArrangedAbsY = y + elem->ArrangedRelY + ClientY;
			}
			else if (IsBottom(Alignment))
			{
				elem->ArrangedAbsY = y + ArrangedContainerHeight - elem->ArrangedHeight - elem->ArrangedRelY - ClientY;
			}
			else
			{
				elem->ArrangedAbsY = y + ArrangedContainerHeight / 2 - ArrangedContentsHeight / 2 + elem->ArrangedRelY;
			}
			elem->ArrangeSubElementsAbsPos(elem->ArrangedAbsX, elem->ArrangedAbsY);
		}
	}

	void UIElementBase::ArrangeElements(int x, int y, int w, int h)
	{
		int cw, ch;
		GetClientContentsSize(w, h, cw, ch);
		ArrangeSubElementsAbsPos(x, y);
	}

	void UIElementBase::Render(int x, int y, int w, int h)
	{
		int ArrangedAbsR = ArrangedAbsX + ArrangedWidth - 1;
		int ArrangedAbsB = ArrangedAbsY + ArrangedHeight - 1;

		if (!Transparent)
		{
			int FillX = ArrangedAbsX + XMargin + XBorder + 1;
			int FillY = ArrangedAbsY + YMargin + YBorder + 1;
			int FillR = ArrangedAbsR - XMargin - XBorder - 1;
			int FillB = ArrangedAbsB - YMargin - YBorder - 1;
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
			int BorderLR = BorderLX + XBorder - 1;
			int BorderLB = ArrangedAbsB - YMargin - YBorder;
			int BorderRX = ArrangedAbsR - XMargin - XBorder + 1;
			int BorderRY = BorderLY;
			int BorderRR = ArrangedAbsR - XMargin;
			int BorderRB = BorderLB;
			int BorderLW = BorderLR - BorderLX + 1;
			int BorderLH = BorderLB - BorderLY + 1;
			int BorderRW = BorderRR - BorderRX + 1;
			int BorderRH = BorderRB - BorderRY + 1;
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
			int BorderTX = BorderLX;
			int BorderTY = BorderLY - YBorder;
			int BorderTR = BorderRR;
			int BorderTB = BorderLY - 1;
			int BorderBX = BorderLX;
			int BorderBY = BorderLB + 1;
			int BorderBR = BorderRR;
			int BorderBB = BorderRB + YBorder;
			int BorderTW = BorderTR - BorderTX + 1;
			int BorderTH = BorderTB - BorderTY + 1;
			int BorderBW = BorderBR - BorderBX + 1;
			int BorderBH = BorderBB - BorderBY + 1;
			if (YBorder > 0)
			{
				if (BorderTW > 0 && BorderTH > 0)
				{
					FB.FillRect(BorderTX, BorderTY, BorderTR, BorderTB, BorderColor);
				}
				if (BorderBW > 0 && BorderBH > 0)
				{
					FB.FillRect(BorderBX, BorderBY, BorderBR, BorderBB, BorderColor);
				}
			}
		}

		int ClientX = ArrangedAbsX + GetFrameWidth();
		int ClientY = ArrangedAbsY + GetFrameHeight();
		int ClientR = ArrangedAbsR - GetFrameWidth();
		int ClientB = ArrangedAbsB - GetFrameHeight();
		if (ClientR >= FB.GetWidth()) ClientR = FB.GetWidth() - 1;
		if (ClientB >= FB.GetHeight()) ClientB = FB.GetHeight() - 1;
		int ClientW = ClientR - ClientX;
		int ClientH = ClientB - ClientY;
		if (ClientW > 0 && ClientH > 0)
		{
			if (ClipChildren)
			{
				auto ImageMask = FB.ReadPixels();
				auto RectAreaImage = ImageBlock(FB.GetWidth(), FB.GetHeight(), 0);
				for (int y = ClientY; y <= ClientB; y++)
				{
					for (int x = ClientX; x <= ClientR; x++)
					{
						RectAreaImage.PutPixel(x, y, 0xFFFFFFFF);
						ImageMask.PutPixel(x, y, 0);
					}
				}
				for (auto& elem : SubElements)
				{
					elem->Render(elem->ArrangedAbsX, elem->ArrangedAbsY, elem->ArrangedWidth, elem->ArrangedHeight);
				}
				FB.DrawImageAnd(RectAreaImage, 0, 0);
				FB.DrawImageOr(ImageMask, 0, 0);
			}
			else
			{
				for (auto& elem : SubElements)
				{
					elem->Render(elem->ArrangedAbsX, elem->ArrangedAbsY, elem->ArrangedWidth, elem->ArrangedHeight);
				}
			}
		}
	}

	void UIElementBase::Render()
	{
		Render(0, 0, FB.GetWidth(), FB.GetHeight());
	}

	std::shared_ptr<UIElementBase> UIElementBase::GetElementByName(const std::string& Name)
	{
		return SubElementsMap.at(Name);
	}

	UIElementBase& UIElementBase::InsertElement(std::shared_ptr<UIElementBase> Element)
	{
		RemoveElement(Element->Name);
		SubElementsMap[Element->Name] = Element;
		SubElements.push_back(Element);
		return *Element;
	}

	bool UIElementBase::RemoveElement(const std::string& Name)
	{
		if (SubElementsMap.count(Name))
		{
			for (auto Element = SubElements.begin(); Element != SubElements.end();)
			{
				if (Element->get()->Name == Name) SubElements.erase(Element);
				else ++Element;
			}
			SubElementsMap.erase(Name);
			return true;
		}
		return false;
	}

	void UIElementBase::ClearElements()
	{
		SubElementsMap.clear();
		SubElements.clear();
	}

	decltype(UIElementBase::SubElements.cbegin()) UIElementBase::cbegin() const
	{
		return SubElements.cbegin();
	}

	decltype(UIElementBase::SubElements.cend()) UIElementBase::cend() const
	{
		return SubElements.cend();
	}

	decltype(UIElementBase::SubElements.begin()) UIElementBase::begin()
	{
		return SubElements.begin();
	}

	decltype(UIElementBase::SubElements.end()) UIElementBase::end()
	{
		return SubElements.end();
	}

	decltype(UIElementBase::SubElements.size()) UIElementBase::size() const
	{
		return SubElements.size();
	}

	decltype(UIElementBase::SubElements.front()) UIElementBase::front()
	{
		return SubElements.front();
	}

	decltype(UIElementBase::SubElements.back()) UIElementBase::back()
	{
		return SubElements.back();
	}

	decltype(UIElementBase::SubElements.at(size_t(0))) UIElementBase::at(size_t Index)
	{
		return SubElements.at(Index);
	}

	decltype(UIElementBase::SubElementsMap.at("")) UIElementBase::at(const std::string& Name)
	{
		return SubElementsMap.at(Name);
	}

	UIElementLabel::UIElementLabel(Graphics& FB, const std::string& Name) :
		UIElementBase(FB, Name)
	{
	}

	void UIElementLabel::GetClientContentsSize(int WidthLimit, int HeightLimit, int& ActualWidth, int& TotalHeight)
	{
		int TextW = CaptionWidth + XPadding * 2;
		int TextH = CaptionHeight + YPadding * 2;
		UIElementBase::GetClientContentsSize(WidthLimit, HeightLimit, ActualWidth, TotalHeight);
		if (ActualWidth < TextW) ActualWidth = TextW;
		if (TotalHeight < TextH) TotalHeight = TextH;
		ArrangedContentsWidth = ActualWidth;
		ArrangedContentsHeight = TotalHeight;
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

		int tw = CaptionWidth, th = CaptionHeight;

		int tx, ty;
		if (IsLeft(Alignment))
		{
			tx = x + GetFrameWidth();
		}
		else if (IsRight(Alignment))
		{
			tx = x + ArrangedWidth - GetFrameWidth() - tw;
		}
		else
		{
			tx = x + w / 2 - tw / 2;
		}
		if (IsTop(Alignment))
		{
			ty = y + GetFrameHeight();
		}
		else if (IsBottom(Alignment))
		{
			ty = y + ArrangedHeight - GetFrameHeight() - th;
		}
		else
		{
			ty = y + h / 2 - th / 2;
		}
		FB.DrawText(tx, ty, Caption, true, FontColor);
	}

	int UIElementListView::GetMaxScroll() const
	{
		int MaxScroll = ArrangedHeight - ArrangedContentsHeight;
		if (MaxScroll < 0) MaxScroll = 0;
		return MaxScroll;
	}

	void UIElementListView::EnsureSelectedVisible()
	{
		int SelectionPositionTop = 0;
		int SelectionPositionBtm = 0;
		for (int i = 0; i < Selection; i++)
		{
			SelectionPositionTop += SubElements[i]->ArrangedHeight;
			SelectionPositionBtm = SelectionPositionTop + SubElements[i]->ArrangedHeight;
		}
		if (SelectionPositionTop + Scroll < 0)
		{
			Scroll = -SelectionPositionTop;
		}
		if (SelectionPositionBtm + Scroll > ArrangedHeight)
		{
			Scroll = -SelectionPositionBtm;
		}
	}

	UIElementListView::UIElementListView(Graphics& FB, const std::string& Name) :
		UIElementBase(FB, Name)
	{
		ClipChildren = true;
	}

	void UIElementListView::GetClientContentsSize(int WidthLimit, int HeightLimit, int& ActualWidth, int& TotalHeight)
	{
		UIElementBase::GetClientContentsSize(WidthLimit, HeightLimit, ActualWidth, TotalHeight);
	}

	size_t UIElementListView::AddItem(const std::string& Key, const std::string& Caption)
	{
		auto elem = std::make_shared<UIElementListItem>(FB, Key);
		InsertElement(elem);
		elem->ExpandToParentX = true;
		elem->LineBreak = true;
		elem->XMargin = 1;
		elem->YMargin = 1;
		elem->XBorder = 1;
		elem->YBorder = 1;
		elem->XPadding = 1;
		elem->YPadding = 1;
		elem->Transparent = false;
		elem->BorderColor = 0xFFA0A0A0;
		elem->FillColor = 0xFF000000;
		elem->Alignment = AlignmentType::LeftCenter;
		elem->SetCaption(Caption);

		if (size() == 1)
		{
			Selection = 0;
			elem->Selected = true;
		}
		return size();
	}

	bool UIElementListView::RemoteItem(size_t Index)
	{
		if (size() == 0) return false;
		if (Index >= size()) return false;
		auto& ElemName = SubElements[Index]->GetName();
		return RemoveElement(ElemName);
	}

	UIElementListItem& UIElementListView::GetItem(size_t Index) const
	{
		if (Index < size())
		{
			return dynamic_cast<UIElementListItem&>(*SubElements[Index]);
		}
		throw std::invalid_argument(std::string(__func__) + ": Index out of bound: index=" + std::to_string(Index) + ", bound=" + std::to_string(size()));
	}

	UIElementListItem& UIElementListView::GetSelectedItem() const
	{
		if (Selection < size())
		{
			return dynamic_cast<UIElementListItem&>(*SubElements[Selection]);
		}
		throw std::invalid_argument(std::string(__func__) + ": Index out of bound: index=" + std::to_string(Selection) + ", bound=" + std::to_string(size()));
	}

	void UIElementListView::SelectNext()
	{
		if (!size()) return;
		GetSelectedItem().Selected = false;
		auto Prev = Selection++;
		if (Selection >= size()) Selection = 0;
		GetSelectedItem().Selected = true;
		EnsureSelectedVisible();
	}

	void UIElementListView::SelectPrev()
	{
		if (!size()) return;
		GetSelectedItem().Selected = false;
		auto Prev = Selection;
		if (Selection == 0) Selection = size();
		Selection--;
		GetSelectedItem().Selected = true;
		EnsureSelectedVisible();
	}

	void UIElementListView::Render(int x, int y, int w, int h)
	{
		UIElementBase::Render(x - Scroll, y, w, h);
	}

	UIElementListItem::UIElementListItem(Graphics& FB, const std::string& Name) :
		UIElementLabel(FB, Name)
	{
	}

	void UIElementListItem::Render(int x, int y, int w, int h)
	{
		UIElementLabel::Render(x, y, w, h);

		if (Selected)
		{
			int FillX = x + XMargin + XBorder + 1;
			int FillY = y + YMargin + YBorder + 1;
			int FillR = x + ArrangedWidth - 1 - XMargin - XBorder - 1;
			int FillB = y + ArrangedHeight - 1 - YMargin - YBorder - 1;
			FB.DrawRectXor(FillX, FillY, FillR, FillB);
		}
	}
}
