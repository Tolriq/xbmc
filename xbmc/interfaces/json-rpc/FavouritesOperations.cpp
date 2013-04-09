/*
 *      Copyright (C) 2011-2013 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "FavouritesOperations.h"
#include "Favourites.h"
#include "input/ButtonTranslator.h"
#include "utils/RegExp.h"

using namespace JSONRPC;

JSONRPC_STATUS CFavouritesOperations::GetFavourites(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CFileItemList favourites;
  CFavourites::Load(favourites);
  
  std::string type = !parameterObject["type"].isNull() ? parameterObject["type"].asString() : "";

  CRegExp windowReg;
  windowReg.RegComp("ActivateWindow\\((\\d*),\"(.*)\"\\)");
  CRegExp mediaReg;
  mediaReg.RegComp("PlayMedia\\(\"(.*)\"\\)");
  CRegExp scriptReg;
  scriptReg.RegComp("RunScript\\(\"(.*)\"\\)");

  std::set<std::string> fields;
  if (parameterObject.isMember("properties") && parameterObject["properties"].isArray())
  {
    for (CVariant::const_iterator_array field = parameterObject["properties"].begin_array(); field != parameterObject["properties"].end_array(); field++)
      fields.insert(field->asString());
  }

  for (int i = 0; i < favourites.Size(); i++)
  {
    CVariant object;
    CFileItemPtr item = favourites.Get(i);

    object["title"] = item->GetLabel();
    if (fields.find("thumbnail") !=  fields.end())
      object["thumbnail"] = item->GetArt("thumb");

    if (windowReg.RegFind(item->GetPath()) != -1)
    {
      object["type"] = "window";
      if (fields.find("window") !=  fields.end())
        object["window"] = CButtonTranslator::TranslateWindow(atoi(windowReg.GetMatch(1).c_str()));
      if (fields.find("windowparameter") !=  fields.end())
        object["windowparameter"] = windowReg.GetMatch(2);
    } 
    else if (mediaReg.RegFind(item->GetPath()) != -1)
    {
      object["type"] = "media";
      if (fields.find("path") !=  fields.end())
        object["path"] = mediaReg.GetMatch(1);
    } 
    else if (scriptReg.RegFind(item->GetPath()) != -1)
    {
      object["type"] = "script";
      if (fields.find("path") !=  fields.end())
        object["path"] = scriptReg.GetMatch(1);
    } else {
      object["type"] = "unknown";
    }
    
    if (type.empty() || type.compare(object["type"].asString()) == 0)
      result["favourites"].append(object);
  }
  
  int start, end;
  HandleLimits(parameterObject, result, result["favourites"].size(), start, end);

  return OK;
}
