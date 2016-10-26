/***********************************************************************************
**    Copyright (C) 2016  Petref Saraci
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You have received a copy of the GNU General Public License
**    along with this program. See LICENSE.GPLv3
**    A copy of the license can be found also here <http://www.gnu.org/licenses/>.
**
************************************************************************************/

import QtQuick 2.7
import Risip 1.0

LoginPageForm {
    id: root

    property RisipAccount sipAccount

    signal signedIn
    signal addSipService

    addSipServiceButton.onClicked: { root.addSipService(); }

    loginButton.onClicked: { sipAccount.login(); }

    Connections {
        target: sipAccount

        onStatusChanged: {
            if(sipAccount.status === RisipAccount.SignedIn) {
                root.signedIn();
            }
        }
    }

    sipServicesInput.model: Risip.accountNames

    sipServicesInput.onActivated: {
        sipAccount = Risip.accountForUri(sipServicesInput.currentText);
        uernameInput.text = sipAccount.configuration.userName
        passwordInput.text = sipAccount.configuration.password

        Risip.setDefaultAccount(sipAccount.configuration.uri);
    }
}
