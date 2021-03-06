/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XMessageBox;
import com.sun.star.awt.XMessageBoxFactory;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.NamedValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.ui.dialogs.XExecutableDialog;
import com.sun.star.ui.dialogs.XFolderPicker;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.CloseVetoException;
import com.sun.star.util.XChangesBatch;
import com.sun.star.util.XCloseable;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;
import java.util.Vector;
import org.openoffice.XInstanceInspector;


/** The purpose of this class is to provide a factory for creating the service
 * (<CODE>__getServiceFactory</CODE>) and writes the information into the given
 * registry key (<CODE>__writeRegistryServiceInfo</CODE>).
 */
public class Inspector{
     public static final String sIDLDOCUMENTSUBFOLDER = "docs/common/ref/";

    
    /** This class implements the method of the interface XInstanceInspector.
     * Also the class implements the interfaces XServiceInfo, and XTypeProvider.
     */
    static public class _Inspector extends WeakBase implements XInstanceInspector, XServiceInfo{
    
        static private final String __serviceName = "org.openoffice.InstanceInspector";    
        private HashMap aApplicationHashMap = new HashMap();
        private String sTitle = "Object Inspector";
        private Vector aHiddenDocuments = new Vector();
//        private String[] sApplicationDocUrls = new String[]{"private:factory/swriter", "private:factory/scalc", "private:factory/simpress", "private:factory/sdraw", "private:factory/sbase"};
//        private String[] sApplicationDocNames = new String[]{"Text Document", "Spreadsheet", "Presentation", "Drawing", "Database"};
        private XComponentContext m_xComponentContext; 
        private HashMap aInspectorPanes = new HashMap();
        private XDialogProvider m_oSwingDialogProvider;
        private TDocSupplier oTDocSupplier;
        private Introspector m_oIntrospector = null;
        // TODO: improve these strings:
        private static final String sWRONGINSTALLATIONPATH = "Your selected path does not refer to an SDK-Installation!";
        /** Creates a new instance of Dialog */
        public _Inspector(XComponentContext _xComponentContext) {
            m_xComponentContext = _xComponentContext;
            m_oIntrospector = Introspector.getIntrospector(m_xComponentContext);
            aApplicationHashMap.put("private:factory/swriter", "Text Document");        
            aApplicationHashMap.put("private:factory/scalc", "Spreadsheet");
            aApplicationHashMap.put("private:factory/simpress", "Presentation");
            aApplicationHashMap.put("private:factory/sdraw", "Drawing");
            aApplicationHashMap.put("private:factory/smath", "Formula");
            m_oSwingDialogProvider = new SwingDialogProvider(this, sTitle);
//            aApplicationHashMap.put("private:factory/sbase", "Database");  
        }
        
        
        public XComponentContext getXComponentContext(){
            return m_xComponentContext;
        }
                        
        
        public HashMap getInspectorPages(){
            return aInspectorPanes;
        }
        
        
        protected String getSDKPath(){
        String sRetPath = "";
        try{
            XNameAccess xNameAccess  = getConfigurationAccess("org.openoffice.inspector.ObjectInspector", true);
            XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xNameAccess);
            sRetPath = (String) xPropertySet.getPropertyValue("SDKPath");   
        }catch( Exception exception ) {
            exception.printStackTrace(System.out);
        }
        return sRetPath;
        }
        
        
        public String getIDLPath(){
            String sRetPath = getSDKPath();
            if (m_oIntrospector.isValidSDKInstallationPath(sRetPath)){
                sRetPath = m_oIntrospector.addToPath(sRetPath, sIDLDOCUMENTSUBFOLDER);
            }
            else{
                sRetPath = "";
            }
            return sRetPath;
        }
        
        
        public void openIdlFileforSelectedNode(){
            InspectorPane oInspectorPane = m_oSwingDialogProvider.getSelectedInspectorPage();
            if (oInspectorPane != null){
                XUnoNode oUnoNode = oInspectorPane.getSelectedNode();
                if (oUnoNode != null){
                    String sPath = getIDLPath();
                    oUnoNode.openIdlDescription(sPath);
                }
            }
        }        
                
                
        public void assignSDKPath() {
        try {
            String sInstallationFolder = "";
            Object oFolderPicker = m_xComponentContext.getServiceManager().createInstanceWithContext("com.sun.star.ui.dialogs.FolderPicker", m_xComponentContext);
            XFolderPicker xFolderPicker = (XFolderPicker) UnoRuntime.queryInterface(XFolderPicker.class, oFolderPicker);
            XExecutableDialog xExecutable = (XExecutableDialog) UnoRuntime.queryInterface(XExecutableDialog.class, oFolderPicker);
            XComponent xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, oFolderPicker);
            String sPath = getSDKPath();
            if (!sPath.equals("")){
                xFolderPicker.setDisplayDirectory(sPath);
            }
            xFolderPicker.setTitle("Add the Path to your SDK installation");
            short nResult = xExecutable.execute();
            if (nResult == com.sun.star.ui.dialogs.ExecutableDialogResults.OK){
                sInstallationFolder = xFolderPicker.getDirectory();
                if (m_oIntrospector.isValidSDKInstallationPath(sInstallationFolder)){
                    XNameAccess xNameAccess = getConfigurationAccess(true);
                    XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xNameAccess);
                    xPropertySet.setPropertyValue("SDKPath", sInstallationFolder);
                    XChangesBatch xBatch = (XChangesBatch) UnoRuntime.queryInterface(XChangesBatch.class, xNameAccess);
                    xBatch.commitChanges();                
                }
                else{
                    XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xFolderPicker);
                    Object oWindow = xPropertySet.getPropertyValue("Window");
                    XWindowPeer xWindowPeer = (XWindowPeer) UnoRuntime.queryInterface(XWindowPeer.class, oWindow);
                    showErrorMessageBox(xWindowPeer, sTitle, sWRONGINSTALLATIONPATH);
                    assignSDKPath();
                }
            }
            xComponent.dispose();
        }catch( Exception exception ) {
            exception.printStackTrace(System.out);
        }}
            
        
        public void showErrorMessageBox(XWindowPeer _xWindowPeer, String _sTitle, String _sMessage){
        try {
            Object oToolkit = m_xComponentContext.getServiceManager().createInstanceWithContext("com.sun.star.awt.Toolkit", m_xComponentContext);
            XMessageBoxFactory xMessageBoxFactory = (XMessageBoxFactory) UnoRuntime.queryInterface(XMessageBoxFactory.class, oToolkit);
            Rectangle aRectangle = new Rectangle();
            XMessageBox xMessageBox = xMessageBoxFactory.createMessageBox(_xWindowPeer, aRectangle, "errorbox", com.sun.star.awt.MessageBoxButtons.BUTTONS_OK, _sTitle, _sMessage);
            XComponent xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, xMessageBox);
            if (xMessageBox != null){
                short nResult = xMessageBox.execute();
                xComponent.dispose();
            }
        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace(System.out);
        }}
        
                
        public void inspect(java.lang.Object _oUserDefinedObject, String _sTitle) throws com.sun.star.uno.RuntimeException {
        try {
            int nPageIndex = m_oSwingDialogProvider.getInspectorPageCount();
            SwingTreeControlProvider oSwingTreeControlProvider = new SwingTreeControlProvider(m_oSwingDialogProvider);
            InspectorPane oInspectorPane = new InspectorPane(getXComponentContext(), m_oSwingDialogProvider, oSwingTreeControlProvider, getSourceCodeLanguage());
            oInspectorPane.inspect(_oUserDefinedObject, _sTitle);
            getInspectorPages().put(_sTitle, oInspectorPane);
            m_oSwingDialogProvider.show(nPageIndex);
        }catch( Exception exception ) {
            exception.printStackTrace(System.out);
        }}
                
        
        public void inspectOpenEmptyDocument(String _sApplicationDocUrl){
            XComponent xComponent = getTDocSupplier().openEmptyDocument(_sApplicationDocUrl);
            String sRootTitle = (String) aApplicationHashMap.get(_sApplicationDocUrl);
            inspect(xComponent, sRootTitle);
            aHiddenDocuments.add(xComponent);
        }
        
        
        public void inspectOpenDocument(String _sTDocUrl){
            String sTreeNodeName = getTDocSupplier().getTitleByTDocUrl(_sTDocUrl);
            XModel xTDocModel = getTDocSupplier().getXModelByTDocUrl(_sTDocUrl);
            inspect(xTDocModel, sTreeNodeName);
        }        
        
        
        public void inspectSelectedNode(){
            InspectorPane oInspectorPane = m_oSwingDialogProvider.getSelectedInspectorPage();
            if (oInspectorPane != null){
                XUnoNode oUnoNode = oInspectorPane.getSelectedNode();
                Object oUnoObject = oUnoNode.getUnoObject();
                if (oUnoObject != null){
                    String sNodeDescription = UnoNode.getNodeDescription(oUnoObject);
                    inspect(oUnoObject, sNodeDescription);
                }
            }
        }

        
        public void addSourceCodeOfSelectedNode(){
            InspectorPane oInspectorPane = m_oSwingDialogProvider.getSelectedInspectorPage();
            if (oInspectorPane != null){
                oInspectorPane.addSourceCodeOfSelectedNode();
            }
        }
        

        public void invokeSelectedMethod(){
            InspectorPane oInspectorPane = m_oSwingDialogProvider.getSelectedInspectorPage();
            if (oInspectorPane != null){
                oInspectorPane.invokeSelectedMethodNode();
            }
        }
                
        
        public void setSourceCodeLanguage(final int _nLanguage){
        try{
            String sLanguage = "Java";
            XNameAccess xNameAccess  = getConfigurationAccess("org.openoffice.inspector.ObjectInspector", true);
            XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xNameAccess);
            switch (_nLanguage){
                case XLanguageSourceCodeGenerator.nJAVA:
                    sLanguage = "Java";
                    break;
                case XLanguageSourceCodeGenerator.nCPLUSPLUS:
                    sLanguage = "CPlusPlus";
                    break;
                case XLanguageSourceCodeGenerator.nBASIC:
                    sLanguage = "Basic";
                    break;
                default:
                    System.out.println("Warning: Sourcecode language is not defined!");
            }
            xPropertySet.setPropertyValue("Language", sLanguage);
            XChangesBatch xBatch = (XChangesBatch) UnoRuntime.queryInterface(XChangesBatch.class, xNameAccess);
            xBatch.commitChanges();
            for (int i = 0; i < m_oSwingDialogProvider.getInspectorPageCount(); i++){
                m_oSwingDialogProvider.getInspectorPage(i).convertCompleteSourceCode(_nLanguage);
            }
        }catch( Exception exception ) {
            exception.printStackTrace(System.out);
        }}
            
        
        private TDocSupplier getTDocSupplier(){
            if (oTDocSupplier == null){
                oTDocSupplier = new TDocSupplier(m_xComponentContext);        
            }
            return oTDocSupplier;
        }

        public String[] getTDocUrls(){
            return getTDocSupplier().getTDocUrls();
        }
        
        
        public String[] getTDocTitles(String[] _sTDocUrls){
            return getTDocSupplier().getTDocTitles(_sTDocUrls);
        }
      
        
        public String[][] getApplicationUrls(){
            Set aSet = aApplicationHashMap.keySet();
            String[][] sReturnList = new String[aSet.size()][];
            int n= 0;
            for ( Iterator i = aSet.iterator(); i.hasNext(); ){
                String[] sSingleApplication = new String[2];
                sSingleApplication[0] = (String) i.next();
                // assign the title in the second index
                sSingleApplication[1] = (String) aApplicationHashMap.get(sSingleApplication[0]);
                sReturnList[n++] = sSingleApplication;
            }
            return sReturnList;
        }

        
        public void disposeHiddenDocuments(){
            int nHiddenCount = aHiddenDocuments.size();
            if (nHiddenCount > 0){
                for (int i = nHiddenCount - 1; i >= 0; i--){
                    XComponent xComponent = (XComponent) aHiddenDocuments.get(i);
                    if (xComponent != null){
                        try {
                            XCloseable xCloseable = (XCloseable) UnoRuntime.queryInterface(XCloseable.class, xComponent);
                            xCloseable.close(true);
                            aHiddenDocuments.remove(i);
                        } catch (CloseVetoException ex) {
                            ex.printStackTrace();
                        }
                    }
                }
            }
        }
        
        
        public static String[] getServiceNames() {
            String[] sSupportedServiceNames = { __serviceName };
            return sSupportedServiceNames;
        }
        
        // Implement the interface XServiceInfo
        /** Get all supported service names.
         * @return Supported service names.
         */
        public String[] getSupportedServiceNames() {
            return getServiceNames();
        }
        
        // Implement the interface XServiceInfo
        /** Test, if the given service will be supported.
         * @param sService Service name.
         * @return Return true, if the service will be supported.
         */
        public boolean supportsService( String sServiceName ) {
            return sServiceName.equals( __serviceName );
        }
        
        // Implement the interface XServiceInfo
        /** Get the implementation name of the component.
         * @return Implementation name of the component.
         */
        public String getImplementationName() {
            return _Inspector.class.getName();
        }
        

        private int getSourceCodeLanguage(){
        int nLanguage = XLanguageSourceCodeGenerator.nJAVA;
        try{
            XNameAccess xNameAccess  = getConfigurationAccess("org.openoffice.inspector.ObjectInspector", false);
            String sLanguage = (String) xNameAccess.getByName("Language");
            if (sLanguage.toUpperCase().equals("JAVA")){
                nLanguage = XLanguageSourceCodeGenerator.nJAVA;
            }
            else if (sLanguage.toUpperCase().equals("BASIC")){
                nLanguage = XLanguageSourceCodeGenerator.nBASIC;                
            }
            else if (sLanguage.toUpperCase().equals("CPLUSPLUS")){
                nLanguage = XLanguageSourceCodeGenerator.nCPLUSPLUS;                
            }
            else{
                System.out.println("Warning: Sourcecode language " + sLanguage + " is not defined!");
            }
            m_oSwingDialogProvider.selectSourceCodeLanguage(nLanguage);
        }catch( Exception exception ) {
            exception.printStackTrace(System.out);
        }
            return nLanguage;
        }            
     
        
        public XNameAccess getConfigurationAccess(boolean _bUpdate){
            return getConfigurationAccess("org.openoffice.inspector.ObjectInspector", _bUpdate);
        }


        public XNameAccess getConfigurationAccess(String _sNodePath, boolean update) {
        XNameAccess xNameAccess = null;
        try {
            String sAccess = "";
            if (update) {
                sAccess = "com.sun.star.configuration.ConfigurationUpdateAccess";
            }
            else{
                sAccess = "com.sun.star.configuration.ConfigurationAccess";
            }
            XMultiComponentFactory xMCF = m_xComponentContext.getServiceManager();
            Object oDefaultProvider = xMCF.createInstanceWithContext("com.sun.star.configuration.DefaultProvider", this.getXComponentContext());
            XMultiServiceFactory xMSFCfg = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, oDefaultProvider);        
            Object oAccess = xMSFCfg.createInstanceWithArguments(sAccess, new Object[]{new NamedValue("nodepath", _sNodePath)});
            xNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oAccess);
        } catch (com.sun.star.uno.Exception e) {  
        }
        return xNameAccess;
        }
        
        
    }
    
// end of inner class
    
    
    /**
     * Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleComponentFactory</code> for creating
     *          the component
     * @param   sImplName the name of the implementation for which a
     *          service is desired
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleComponentFactory __getComponentFactory( String sImplName )
    {
        XSingleComponentFactory xFactory = null;
        if ( sImplName.equals( _Inspector.class.getName() ) )
            xFactory = Factory.createComponentFactory(_Inspector.class, _Inspector.getServiceNames());
        if ( xFactory == null )
            xFactory = InspectorAddon.__getComponentFactory(sImplName);
        return xFactory;
    }

    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns true if the operation succeeded
     * @param   regKey the registryKey
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return (Factory.writeRegistryServiceInfo(_Inspector.class.getName(), _Inspector.getServiceNames(), regKey) 
                && InspectorAddon.__writeRegistryServiceInfo(regKey));
    }
}
    
