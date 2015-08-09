/***************************************************************************
 * Copyright (C) 2015 by Pablo Daniel Pareja Obregon                       *
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

#include "fileexport.h"

#include "cgraphicsscene.h"
#include "component.h"
#include "idocument.h"
#include "portsymbol.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

namespace Caneda
{
    //! \brief Constructor.
    FormatSpice::FormatSpice(SchematicDocument *doc) :
        m_schematicDocument(doc)
    {
    }

    bool FormatSpice::save()
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!scene) {
            return false;
        }

        QFile file(fileName());
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot save document!"));
            return false;
        }

        QString text = generateNetlist();
        if(text.isEmpty()) {
            qDebug("Looks buggy! Null data to save! Was this expected?");
        }

        QTextStream stream(&file);
        stream << text;
        file.close();

        return true;
    }

    SchematicDocument *FormatSpice::schematicDocument() const
    {
        return m_schematicDocument;
    }

    CGraphicsScene *FormatSpice::cGraphicsScene() const
    {
        return m_schematicDocument ? m_schematicDocument->cGraphicsScene() : 0;
    }

    QString FormatSpice::fileName() const
    {
        if(m_schematicDocument) {
            QFileInfo info(m_schematicDocument->fileName());
            QString baseName = info.completeBaseName();
            QString path = info.path();

            return path + "/" + baseName + ".net";
        }

        return QString();
    }

    /*!
     *  \brief Generate netlist
     *
     *  Iterate over all components, saving to a string the schematic netlist
     *  according to the model provided as a set of rules. In order to do so,
     *  the netlist topology must also be created, that is the connections
     *  between the multiple components must be determined and numbered to be
     *  used for the spice netlist.
     *
     *  Each "part" or "block" of a spice model is separated by spaces. Each
     *  block begins with a "%", which is a command indicating what goes next,
     *  followed optionaly by a "=" indicating an item or name of a group. For
     *  example, a block may be %port=A indicating that a port must be added,
     *  and in particular of all ports, the port A must be written. If not "%"
     *  is given, the text must be copied "as is".
     *
     *  \sa generateNetlistTopology()
     */
    QString FormatSpice::generateNetlist()
    {
        QList<QGraphicsItem*> items = cGraphicsScene()->items();
        QList<Component*> components = filterItems<Component>(items);
        QList<QPair<Port *, int> > netlist = generateNetlistTopology();

        // Fist we start the document and write the header
        QString retVal;
        retVal.append("* Spice automatic export. Generated by Caneda.\n");

        // Now copy all the elements and properties in the schematic
        // Iterate over all schematic components
        foreach(Component *c, components) {

            // Get the spice model (multiple models may be available)
            QString model = c->model("spice");

            QStringList modelBlocks = model.split(" ", QString::SkipEmptyParts);
            for(int i=0; i<modelBlocks.size(); i++){

                QStringList modelSubBlocks = modelBlocks.at(i).split("%", QString::SkipEmptyParts);
                for(int j=0; j<modelSubBlocks.size(); j++){

                    QStringList modelCommands = modelSubBlocks.at(j).split("=", QString::SkipEmptyParts);
                    if(modelCommands.at(0) == "label"){
                        retVal.append(c->label());
                    }
                    else if(modelCommands.at(0) == "port"){

                        foreach(Port *_port, c->ports()) {
                            if(_port->name() == modelCommands.at(1)) {
                                // Found the port, now look for its netlist number
                                for(int i = 0; i < netlist.size(); ++i) {
                                    if(netlist.at(i).first == _port) {
                                        retVal.append(QString::number(netlist.at(i).second));
                                    }
                                }
                            }
                        }

                    }
                    else if(modelCommands.at(0) == "property"){
                        retVal.append(c->properties()->propertyValue(modelCommands.at(1)));
                    }
                    else{
                        retVal.append(modelSubBlocks.at(j));
                    }

                }
                retVal.append(" ");

            }
            retVal.append("\n");
        }

        return retVal;
    }

    /*!
     *  \brief Generate netlist net numbers
     *
     *  Iterate over all ports, to group all connected ports under
     *  the same name (name = equiId). This name or net number must
     *  be used afterwads by all component ports during netlist
     *  generation.
     *
     *  We use all connected ports (including those connected by wires),
     *  instead of connected wires during netlist generation. This allows
     *  to create a netlist node even on those places not connected by
     *  wires (for example when connecting two components together).
     *
     *  \sa saveComponents(), Port::getEquipotentialPorts()
     */
    QList<QPair<Port *, int> > FormatSpice::generateNetlistTopology()
    {
        //! \todo Generate a type for QList<QPair<Port*, int> > and use that instead
        /*! \todo Investigate: If we use QList<CGraphicsItem*> canedaItems = filterItems<Ports>(items);
         *  some phantom ports appear, and seem to be uninitialized, generating an ugly crash. Hence
         *  we filter generic items and use an iteration over their ports as a workaround.
         */
        QList<QGraphicsItem*> items = cGraphicsScene()->items();
        QList<CGraphicsItem*> canedaItems = filterItems<CGraphicsItem>(items);
        QList<Port*> ports;
        foreach(CGraphicsItem *i, canedaItems) {
            ports << i->ports();
        }

        int equiId = 1;
        QList<QPair<Port*, int> > netlist;
        QList<Port*> parsedPorts;

        foreach(Port *p, ports) {
            if(parsedPorts.contains(p)) {
                continue;
            }

            QList<Port*> equi;
            p->getEquipotentialPorts(equi);
            foreach(Port *_port, equi) {
                netlist.append(qMakePair(_port, equiId));
            }

            equiId++;
            parsedPorts += equi;
        }

        replacePortNames(&netlist);

        return netlist;
    }

    /*!
     * \brief Replace net names in the netlist by those specified by
     * portSymbols.
     *
     * Iterate over all nets in the netlist, replacing those names that
     * correspond to the ones selected by the user using PortSymbols.
     * Take special care of the ground nets, that must be named "0" to
     * be complatible with the spice netlist format.
     *
     * \param netlist Netlist which is to be used in PortSymbol names
     * replacement.
     *
     * \sa PortSymbol, generateNetlistTopology()
     */
    void FormatSpice::replacePortNames(QList<QPair<Port *, int> > *netlist)
    {
        QList<QGraphicsItem*> items = cGraphicsScene()->items();
        QList<PortSymbol*> portSymbols = filterItems<PortSymbol>(items);

        // Iterate over all PortSymbols
        foreach(PortSymbol *p, portSymbols) {

            // Given the port, look for its netlist number
            int netName;
            for(int i = 0; i < netlist->size(); ++i) {
                if(netlist->at(i).first == p->port()) {
                    netName = netlist->at(i).second;
                }
            }

            // Given the netlist number, rename all occurencies with the new name
            for(int i = 0; i < netlist->size(); ++i) {
                if(netlist->at(i).second == netName) {
                    if(p->label().toLower() == "ground" || p->label().toLower() == "gnd") {
                        netlist->replace(i, qMakePair(netlist->at(i).first, 0));
                    }
                    else {
                        //! \todo Replace int by QString to be able to use generic labels.
                        // netlist.at(i).second = p->label();
                    }
                }
            }
        }
    }

} // namespace Caneda
