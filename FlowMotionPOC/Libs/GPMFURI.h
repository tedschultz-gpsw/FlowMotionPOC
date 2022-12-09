/*! @file GPMFURI.h
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

#ifndef GPMF_URI_H
#define GPMF_URI_H

#include <stdio.h>
#include <string>

struct Uri
{
public:
    static Uri Parse(const std::wstring &uri);
    
    std::wstring m_QueryString;
    std::wstring m_Path;
    std::wstring m_Protocol;
    std::wstring m_Host;
    std::wstring m_Port;
    
};  // uri

#endif /* GPMF_URI_H */
