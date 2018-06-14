makeCMS3ClassFiles_2018

### Description:
Creates class files--for CMS3 or anything else--and looper with ht, met_pt, mt2, and nJet30 histograms included.

### Download:

    git clone https://github.com/jackschrott/makeCMS3ClassFiles_2018.git

### Running:

    1.) go to makeClassFiles_2018 directory

          $ cd makeClassFiles_2018
      
    2.) Start ROOT

          $ root
      
    3.) Load makeCMS3ClassFiles

         root[0] .L makeCMS3ClassFiles.C++
      
    4.) Run makeCMS3ClassFiles. * indicates optional arg.
      
         root[1] makeCMS3ClassFiles("<fileName>.root", "<tree in file>", *"<className>", *"<namespaceName>", *"<objectName>")
         
    5.) Exit root
        
         root[2] .q
      
### Looping over Events:

    1.) Loop over all events
    
         $ root doAll.C
         
    * It may take a minute or so for the plots to fully render.
    
