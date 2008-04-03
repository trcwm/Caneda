// Copyright (C) 2008 ROUCARIES Bastien <roucaries.bastien+qucs@gmail.com>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//**************************************************************************
// 

/*
\file QXmlStreamReaderExt.h
\author ROUCARIES Bastien
\brief Implement xml parser extending QXmlStreamReader class
*/

#include <QtXml/QXmlStreamReader>

#include "QRelaxNGvalidator.h"
#include "QXsltTransformer.h"

namespace Qucs {

class QXmlStreamReaderExt :
  public QXmlStreamReader
{
public:
  /*!\brief Default constructor */
  QXmlStreamReaderExt (): QXmlStreamReader() 
  {
    this->bless();
  }
  /* construct from a file name */
  /*
  QXmlStreamReaderExt (const QString &name, const QRelaxNGvalidator * schema = NULL,
		       const QXsltTransformer * xslt = NULL);
  */
  /* construct from memory */
  QXmlStreamReaderExt (const QByteArray &array, const QRelaxNGvalidator * schema = NULL,
		       const QXsltTransformer *xslt = NULL);
  ~QXmlStreamReaderExt();

protected:
  /*!\brief The xml file loaded in memory */
  QByteArray data;
  /*!\brief The xml file output (temporary) */
  void * xmlout;
  
private:
  /*!\brief Default and safe initialisation */
  void bless()
  {
    this->data = NULL;
    this->xmlout = NULL;
  };
  /* finalize construction */
  void finalize(const void *docvoid,
		const QRelaxNGvalidator * schema,
		const QXsltTransformer *xslt);
};

} // namespace qucs