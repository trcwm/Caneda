/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012-2016 by Pablo Daniel Pareja Obregon                  *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "component.h"

#include "library.h"
#include "port.h"
#include "settings.h"
#include "xmlutilities.h"

#include <QDebug>
#include <QFile>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    //! \brief Constructs default empty ComponentData.
    ComponentData::ComponentData()
    {
        properties = new PropertyGroup();
    }

    /*!
     * \brief Copy ComponentData from a ComponentDataPtr.
     *
     * Copies data from a ComponentData pointed by a ComponentDataPtr. Special
     * care is taken to avoid copying the properties pointer, and copying
     * properties content (PropertyMap) instead. Otherwise, one would be
     * copying the reference to the PropertyGroup (properties) and all
     * components would share only one reference, modifying only one set
     * of properties data.
     */
    void ComponentData::setData(const QSharedDataPointer<ComponentData> &other)
    {
        // Copy all data from given ComponentDataPtr
        name = other->name;
        filename = other->filename;
        displayText = other->displayText;
        labelPrefix = other->labelPrefix;
        description = other->description;
        library = other->library;
        ports = other->ports;

        // Recreate PropertyGroup (properties) as it is a pointer
        // and only internal data must be copied.
        properties->setPropertyMap(other->properties->propertyMap());

        models = other->models;
    }

    /*!
     * \brief Constructs and initializes a default empty component item.
     *
     * \param parent Parent of the component item.
     */
    Component::Component(QGraphicsItem *parent) : GraphicsItem(parent)
    {
        // Set component flags
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);

        // Create component shared data
        d = new ComponentData();
        updateSharedData();
    }

    //! \brief Destructor.
    Component::~Component()
    {
        qDeleteAll(m_ports);
    }

    /*!
     * \brief Update this component's shared data related properties.
     *
     * This method updates the component's properties related to its shared
     * data, for example adds the component ports depending on the ports
     * available in the shared data. It also adds an initial label based on the
     * default prefix value.
     */
    void Component::updateSharedData()
    {
        // Add component label
        Property _label("label", labelPrefix().append('1'), QObject::tr("Label"), true);
        d->properties->addProperty("label", _label);

        // Add component ports
        const QList<PortData*> portDatas = d.constData()->ports;
        foreach(const PortData *data, portDatas) {
            Port *port = new Port(this);
            port->setName(data->name);
            port->setPos(data->pos);
            m_ports << port;
        }

        // Update component geometry
        updateBoundingRect();

        // Update properties text position
        d->properties->setParentItem(this);
        d->properties->setTransform(transform().inverted());
        d->properties->setPos(boundingRect().bottomLeft());
    }

    //! \brief Returns the label's suffix part.
    QString Component::labelSuffix() const
    {
        QString _label = label();
        return _label.mid(labelPrefix().length());
    }

    /*!
     * \brief Sets the label of component.
     *
     * This method also handles label prefix and number suffix appropriately.
     * In case the label doesn't start with the correct number prefix, the
     * return value is false.
     *
     * \param newLabel The label to be set.
     * \return True on success and false on failure.
     */
    bool Component::setLabel(const QString& newLabel)
    {
        if(!newLabel.startsWith(labelPrefix())) {
            return false;
        }

        d->properties->setPropertyValue("label", newLabel);
        return true;
    }

    /*!
     * \brief Sets the data of the component.
     *
     * This method also handles updating internal data, component label,
     * component ports, etc.
     *
     * \param other Component data to set into this component.
     */
    void Component::setComponentData(const ComponentDataPtr &other)
    {
        d->setData(other);
        updateSharedData();
    }

    /*!
     * \brief Returns the specified model of a component.
     *
     * Models are the representation of a component in different scenarios.
     * For example, a component can have certain syntax to be used in a spice
     * circuit, and a different one in a kicad schematic. Having a way to
     * extract information from our schematic and interpret it in different
     * ways allow us to export the circuit to other softwares and simulator
     * engines.
     *
     * Models should be always strings. Gouping several models into a QMap
     * provides a convenient way of handling them all together, and filter
     * them according to the export operation being used.
     *
     * \param type The type of model to return (for example, spice).
     * \return QString with the component's model.
     *
     * \sa \ref ModelsFormat.
     */
    QString Component::model(const QString& type) const
    {
        return d->models[type];
    }

    /*!
     * \brief Paints a previously registered component.
     *
     * Takes care of painting a component on a scene. The component must be
     * previously registered using LibraryManager::registerComponent(). This
     * method also takes care of setting the correct global settings pen
     * according to its selection state.
     *
     * \sa LibraryManager::registerComponent()
     */
    void Component::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *)
    {
        // Paint the component symbol
        Settings *settings = Settings::instance();
        LibraryManager *libraryManager = LibraryManager::instance();
        QPainterPath symbol = libraryManager->symbolCache(name(), library());

        // Save pen
        QPen savedPen = painter->pen();

        if(option->state & QStyle::State_Selected) {
            // If selected, the paint is performed without the pixmap cache
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));

            painter->drawPath(symbol);  // Draw symbol
        }
        else if(painter->worldTransform().isScaling()) {
            // If zooming, the paint is performed without the pixmap cache
            painter->setPen(QPen(settings->currentValue("gui/lineColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));

            painter->drawPath(symbol);  // Draw symbol
        }
        else {
            // Else, a pixmap cached is used
            QPixmap pix = libraryManager->pixmapCache(name(), library());
            QRect rect =  symbol.boundingRect().toRect();
            rect.adjust(-1.0, -1.0, 1.0, 1.0);  // Adjust rect to avoid clipping when size = 1px in any dimension
            painter->drawPixmap(rect, pix);
        }

        // Restore pen
        painter->setPen(savedPen);
    }

    //! \copydoc GraphicsItem::copy()
    Component* Component::copy() const
    {
        Component *component = new Component(parentItem());
        component->setComponentData(d);

        GraphicsItem::copyDataTo(component);
        return component;
    }

    /*!
     * \copydoc GraphicsItem::saveData()
     *
     * Saves current component data (name, library, position, properties
     * and transform) to \a Caneda::XmlWriter.
     *
     * \sa loadData()
     */
    void Component::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("component");
        writer->writeAttribute("name", name());
        writer->writeAttribute("library", library());

        writer->writePointAttribute(pos(), "pos");
        writer->writeTransformAttribute(sceneTransform());

        d->properties->writeProperties(writer);

        writer->writeEndElement();  //</component>
    }

    /*!
     * \copydoc GraphicsItem::loadData()
     *
     * Loads current component data (name, library, position, properties
     * and transform) from \a Caneda::XmlReader. Once the component name and
     * library are retrieved, the component data is created from LibraryManager
     * and the remaining properties are read from the
     * PropertyGroup::readProperties() method.
     *
     * \sa saveData()
     */
    void Component::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "component");

        setPos(reader->readPointAttribute("pos"));
        setTransform(reader->readTransformAttribute("transform"));

        QString compName = reader->attributes().value("name").toString();
        QString libName = reader->attributes().value("library").toString();
        ComponentDataPtr data = LibraryManager::instance()->componentData(compName, libName);

        // If the component is found in any Caneda library, copy its data,
        // otherwise read to the end of the file.
        if(data.constData()) {
            setComponentData(data);
        }
        else {
            qWarning() << "Warning: Found unknown element" << compName << ", skipping...";
            reader->readUnknownElement();
            return;
        }

        // Read the component properties
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "properties") {
                    d->properties->readProperties(reader);
                }
                else {
                    qWarning() << "Warning: Found unknown element" << reader->name().toString();
                    reader->readUnknownElement();
                }
            }
        }
    }

    //! \copydoc GraphicsItem::launchPropertiesDialog()
    void Component::launchPropertiesDialog()
    {
        d->properties->launchPropertiesDialog();
    }

    //! \brief Returns the rect adjusted to accomodate ports too.
    QRectF Component::adjustedBoundRect(const QRectF& rect)
    {
        QRectF adjustedRect = rect;
        foreach(Port *port, m_ports) {
            adjustedRect |= portEllipse.translated(port->pos());
        }
        return adjustedRect;
    }

    //! \brief Updates the bounding rect of this item.
    void Component::updateBoundingRect()
    {
        // Get the bounding rect of the symbol
        QPainterPath symbol = LibraryManager::instance()->symbolCache(name(), library());

        // Get an adjusted rect for accomodating extra stuff like ports.
        QRectF adjustedRect = adjustedBoundRect(symbol.boundingRect());

        // Set symbol bounding rect
        setShapeAndBoundRect(QPainterPath(), adjustedRect);
    }

} // namespace Caneda
