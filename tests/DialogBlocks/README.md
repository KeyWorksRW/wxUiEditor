# Import XRC notes

While DialogBlocks was updated in 2020, the underlying code does not appear to have been updated for over a decade. As such, trying to import a wxUiEditor-generated XRC file, saving the project and then trying to import the DialogBlocks project can lose a _lot_ of properties. The write XRC, import XRC, save project, import project should definitely _not_ be used as a definitive way to verify importing works. It's simply a handy way to test importing given the lack of DialogBlocks projects on GitHub.
