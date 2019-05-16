#!/usr/bin/env stack
{- stack
   script
   --resolver lts-13.21
   --package bytestring
   --package unix
-}

import qualified Data.ByteString.Lazy as L
import System.Environment
import System.Exit
import System.Posix.Files
import System.Posix.Types
import Control.Monad


equalSize :: String -> String -> IO Bool
equalSize path1 path2 = do
    size1 <- getFileSize path1
    size2 <- getFileSize path2
    return (size1 == size2)
  where
    getFileSize path = fmap fileSize $ getFileStatus path


equalContent :: String -> String -> IO Bool
equalContent path1 path2 = do
    content1 <- L.readFile path1
    content2 <- L.readFile path2
    return (content1 == content2)


equalFiles :: String -> String -> IO Bool
equalFiles path1 path2 = liftM2 (&&) (equalSize path1 path2) (equalContent path1 path2)


main :: IO ()
main = do
    args <- getArgs
    if (length args) /= 2
    then do
      putStrLn "Usage: juez <file1> <file2>"
      exitWith $ ExitFailure 255
    else do
      let path1 = args !! 0
      let path2 = args !! 1
      areEqual <- equalFiles path1 path2
      if areEqual
      then exitSuccess
      else exitFailure
