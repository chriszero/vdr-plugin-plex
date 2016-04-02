#include "user.h"

namespace plexclient {

    user::user(std::istream *response) {
        try {
            InputSource src(*response);
            DOMParser parser;
            Poco::XML::AutoPtr<Document> pDoc = parser.parse(&src);

            NodeIterator it(pDoc, Poco::XML::NodeFilter::SHOW_ALL);
            Node *pNode = it.nextNode();

            while (pNode) {
                if (Poco::icompare(pNode->nodeName(), "authentication-token") == 0) {
                    pNode = it.nextNode();
                    authenticationToken = pNode->nodeValue();
                    break;
                }
                pNode = it.nextNode();
            }

        } catch (Exception &exc) {
            std::cerr << exc.displayText() << std::endl;
        }

    }

}
