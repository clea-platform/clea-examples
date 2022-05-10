

def clear(layout):
    for item_idx in range(layout.count()):
        item = layout.itemAt(item_idx)
        if item is not None:
            widget = item.widget()
            if widget is not None:
                layout.removeWidget(widget)
                widget.deleteLater()
            else:
                layout = item.layout()
                layout.removeItem(item)
                if layout is not None:
                    clear(layout)
