# HELPERS #

Warfront is free and open source, and always will be. While I will always appreciate help from others, my plan for this project is to rather have you fork your own versions of it and mess around with it as you please. That being said, I'm no expert (especially at C) so I'll be more than welcome to merge in bug fixes or helpful shortcuts. However, there are a few ground rules I want to enforce in order to keep the project consistent and under control.

## Libraries ##

I generally prefer to use as little libraries as possible, in order to keep the project simple. So unless some functionality is both required (or highly demanded) and near impossible to implement by scratch, libraries would suffice. But here's the order to check on whether a library would be needed:

- ### Native C Code ###
  - If the functionality can be replicated or created just in C, please do so. Raylib offers most core game functionality and there shouldn't be a big need to expand upon it. Things such as drawing 2D, 3D, audio, models, file loading, etc. Raylib is very versatile and hopefully it covers most use cases. Anything else hopefully can be coded in by hand.
  
- ### C Library ###
  - If we do absolutely need a library, I much prefer finding an already existing C solution. I want to keep the project on C so I'm not constantly fighting between languages and having to convert or make wrappers. There are plenty of good tools out there in pure C (and even just header libraries) that may be used.
 
- ### C Wrapper Library ###
  - Before I found Raylib, I was working on this from scratch, and I found cglm to wrap glm math calls for C. Obviously that is irrelevant now since raylib can handle the math and rendering. But if there is some event where there's no pure C library, a wrapper will suffice. Wrappers do allow the project to stay in C and prevent continuous conversions or wrappings of our own.
    
- ### C++ or other language libraries ###
  - This should be a last option, and if it gets to this point I assume something is way out of scope. If it ever completely ABSOLUTELY NEEDS to be in the project, a C++ library would be allowed. Most compilers are both C and C++ compatible so there shouldn't be much issue. However, it would require some parts written in C++ which I want to heavily avoid. This would be a last ditch effort and once again, is most likely unnecessary. 
    

## Bug Fixes ##

Nobody's a perfect coder, and I know that well. I am far from an expert at C, so any fixes on bugs are always welcome. Generally I will allow bug fixes, but I will need to manually need to make sure it runs, since I don't think this project will use Actions.

## Optimizations ##

People who code in C tend to know a lot about optimization and best practices. Way more than me. While I am always open to finding better and faster solutions, please make sure the solution does not impede the core functionality. Some things may need to be done because of X Y Z. It will also be very helpful if in a PR for it, a quick summary of what this is optimizing and how it works is made.
