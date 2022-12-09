/*! @file GPMFURI.cpp
 *
 *  @brief libGPMF
 *
 *  @version 1.0.0
 *
 *  (C) Copyright 2019 GoPro Inc (http://gopro.com/).
 *
 *  Licensed under either:
 *  - Apache License, Version 2.0, http://www.apache.org/licenses/LICENSE-2.0
 *  - MIT license, http://opensource.org/licenses/MIT
 *  at your option.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <string>
#include <algorithm>    // find
#include "GPMFURI.h"

Uri Uri::Parse(const std::wstring &uri)
{
    
    Uri result;
    
    typedef std::wstring::const_iterator iterator_t;
    
    if (uri.length() == 0)
        return result;
    
    iterator_t uriEnd = uri.end();
    
    // get query start
    iterator_t queryStart = std::find(uri.begin(), uriEnd, L'?');
    
    // m_Protocol
    iterator_t protocolStart = uri.begin();
    iterator_t protocolEnd = std::find(protocolStart, uriEnd, L':');            //"://");
    
    if (protocolEnd != uriEnd)
    {
        std::wstring prot = &*(protocolEnd);
        if ((prot.length() > 3) && (prot.substr(0, 3) == L"://"))
        {
            result.m_Protocol = std::wstring(protocolStart, protocolEnd);
            protocolEnd += 3;   //      ://
        }
        else
            protocolEnd = uri.begin();  // no m_Protocol
    }
    else
        protocolEnd = uri.begin();  // no m_Protocol
    
    // m_Host
    iterator_t hostStart = protocolEnd;
    iterator_t pathStart = std::find(hostStart, uriEnd, L'/');  // get pathStart
    
    iterator_t hostEnd = std::find(protocolEnd,
                                   (pathStart != uriEnd) ? pathStart : queryStart,
                                   L':');  // check for m_Port
    
    result.m_Host = std::wstring(hostStart, hostEnd);
    
    // m_Port
    if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == L':'))  // we have a m_Port
    {
        hostEnd++;
        iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
        result.m_Port = std::wstring(hostEnd, portEnd);
    }
    
    // m_Path
    if (pathStart != uriEnd)
        result.m_Path = std::wstring(pathStart, queryStart);
    
    // query
    if (queryStart != uriEnd)
        result.m_QueryString = std::wstring(queryStart, uri.end());
    
    return result;
    
}   // Parse
    
